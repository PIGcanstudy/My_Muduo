//
// Created by ubuntu on 24-7-19.
//

#include "EventLoop.h"
#include "Channel.h"
#include "logger.h"
#include <sys/eventfd.h>

//防止一个线程创建多个EventLoop
//当创建了一个EventLoop对象时，*t_loopInThisThread就指向这个对象
//在一个线程里面在创建EventLoop时，指针不为空就不会创建了
//从而控制了一个线程里面只有一个EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

//创建wakeupfd 用来notify唤醒subReactor处理新来的channel
int createEventfd()
{
    //eventfd 计数不为零表示有可读事件发生，read 之后计数会清零，write 则会递增计数器。
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error:%d \n",errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : activeChannels_(0)
    , poller_(Poller::newDefaultPoller(this))
    , looping_(false)
    , quit_(false)
    , pollReturnTime_(0)
    , threadId_(CurrentThread::tid())
    , wakeupFd_(createEventfd())
    , wakeupChannel_(std::make_unique<Channel>(wakeupFd_, this))
{
    LOG_DEBUG("EventLoop created %p in thread %d \n",this threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EvnetLoop %p exists in this thread %d \n",t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    //设置wakeupfd的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));

    //每一个eventloop都将监听wakeupchannel的EPOLLIN读事件了
    //minreactor通过给subreactor写东西，通知其苏醒
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {

    looping_.store(true);
    quit_.store(false);

    LOG_INFO("EventLoop %p start looping \n",this);

    while(!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, activeChannels_);
        for(auto activeChannel: activeChannels_) {
            //poller监听哪些channel发生事件了，然后上报给EventLoop，通知channel处理相应的事件
            activeChannel->handleevent(pollReturnTime_);
        }

        /**
         * IO线程 mainloop accept fd <= channel  subloop
         * mainloop事先注册一个回调cb，需要subloop执行
         * wakeup subloop后执行下面的方法 执行之前mainloop注册的cb回调
         *
        */
        //执行当前EventLoop事件循环需要处理的回调操作
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping,\n",this);
    looping_ = false;
}

//退出事件循环
//1. loop在自己的线程中调用quit
//2. 在其他线程中调用的quit（在一个subloop（woker）中，调用了mainloop（IO）的quit）
/*
                mainloop

    ****************************** 生产者-消费者的线程安全的队列（no）

    subloop1     subloop2     subloop3

*/
void EventLoop::quit() {
    quit_.store(true);

    // 如果是在其它线程中，调用的quit 在一个subloop(woker)中，调用了mainLoop(IO)的quit
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::wakeup() const {
    uint64_t one = 1;
    ssize_t evnums = write(wakeupFd_, &one, sizeof one);
    if(evnums != sizeof one) {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n",evnums);
    }
}

void EventLoop::handleRead() const {
    uint64_t one{};
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8",n);
    }
}

void EventLoop::removechannel(Channel* channel) {
    poller_->removeChannel(channel);
}

void EventLoop::updatechannel(Channel* channel) {
    poller_->updateChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
    // 使用一个局部的vector和pendingFunctors_的交换，有两种好处
    // 1. 可以缩小pendingFunctors_的容量, 因为如果只是resize
    // 它只会重新设定大小（size）而不会重新设定容量
    // 2. 最重要的原因：可以最大的减小占用互斥锁的时间，使得其只在swap加锁
    // 在执行回调函数的时候不加锁，可能还能预防在执行回调函数的时候获取锁而死锁
    // 也能在确保执行回调函数的时候，能够往pendingFunctors_里加数据
    std::vector<Functor> functors;
    pcallingPendingFunctors_.store(true);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    // 此处可以优化，将处理回调函数单独交给一个逻辑系统（线程池）来处理
    // 实现解耦
    for(const Functor& func: functors) {
        func();
    }

    pcallingPendingFunctors_.store(false);
}

// 用来与上处设计比较
/*
void EventLoop::doPendingFunctors() {

    pcallingPendingFunctors_.store(true);
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // 此处可以优化，将处理回调函数单独交给一个逻辑系统（线程池）来处理
        // 实现解耦
        for(const Functor& func: functors) {
            func();
        }
    }

    pcallingPendingFunctors_.store(false);
}
 */

void EventLoop::runInLoop(const Functor &cb) {
    //如果调用的线程是事件循环线程直接调用回调函数
    if(isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor &cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // 当出现以下两种情况的时候进行唤醒
    /** 这是用来唤醒事件循环线程（就是执行EventLoop.loop()的线程）
   // 其设计思想是当另外一个线程，调用了此EventLoop并往里面加入回调函数的时候，唤醒事件循环线程
   // 会有两种唤醒情况
       * 1. 会唤醒被poller_->poll(kPollTimeMs,&activeChannels_);阻塞的事件循环线程
       * 2. 事件循环线程正在执行回调函数，当他执行完后，再次调用poller_->poll(kPollTimeMs,&activeChannels_);
       * 由于有新的事件发生了（eventfd也就是wakeupChannel_有读事件）
       * 就不会被阻塞而继续执行doPendingFunctors();
   **/
    if(!isInLoopThread() || pcallingPendingFunctors_.load()) {
        wakeup();
    }
}






















