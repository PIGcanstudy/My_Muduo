#include "EventLoopThread.h"

EventLoopThread::EventLoopThread(const ThreadInitCallBack &cb, const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::threadFunc, this))
    , mutex_()
    , cond_()
    , callback_(cb)
{

}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if(loop_ != nullptr) {
        // 退出循环
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    // 创建线程, 执行任务
    thread_.start();
    /**
     * 会调用Thread::start()，然后执行func_(); func_(std::move(func))；
     * 而func就是&EventLoopThread::threadFunc,this 传入的，所以会启动一个新线程
    */
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock,[this]() {
            return loop_ != nullptr;
        });
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc() {

    EventLoop loop;

    if(callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::lock_guard<std::mutex> lock(mutex_);
    loop_=nullptr;
}


