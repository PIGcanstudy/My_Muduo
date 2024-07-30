#pragma once

#include <functional>
#include <atomic>
#include <mutex>

#include "EPollPoller.h"
#include "CurrentThread.h"
#include "TimeStamp.h"

// 事件循环类
class EventLoop {
public:
    using Functor = std::function<void()>;
    using ChannelList = std::vector<Channel*>;

    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    //开启事件循环
    void loop();

    //退出事件循环
    void quit();

    //返回处理的时间辍
    TimeStamp pollReturnTime() const { return pollReturnTime_; }

    //EventLoop的方法=> poller的方法
    void updatechannel(Channel* channel);
    void removechannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // 往存放回调函数的所有集合加数据
    void queueInLoop(const Functor& cb);
    void runInLoop(const Functor& cb);

    // 用来唤醒事件循环线程
    void wakeup() const;

    // 证明EventLoop创建时的线程id与当前线程id是否相等
    // 相等表示EventLoop就在所创建他的loop线程里面，可以执行回调
    // 不相等就需要queueInLoop，等待唤醒它自己的线程时，在执行回调
    bool isInLoopThread() const{ return threadId_ == CurrentThread::tid();}
    //[[nodiscard]] EPollPoller* ReturnEPollPoller() const;
private:
    //唤醒用的 wake up
    void handleRead() const;

    //执行回调函数用
    void doPendingFunctors();

    // 用来存放活跃的channels
    ChannelList activeChannels_;

    // 指向poller 来调用他的API
    std::unique_ptr<Poller> poller_;

    // 标志事件循环是否在执行
    std::atomic_bool looping_;

    // 标志事件循环是否停止
    std::atomic_bool quit_;

    // poller返回发生事件的channels的时间点
    TimeStamp pollReturnTime_;

    // 用来记录事件循环线程的tid
    const pid_t threadId_;

    /** 这是用来唤醒事件循环线程（就是执行EventLoop.loop()的线程）
    // 其设计思想是当另外一个线程，调用了此EventLoop并往里面加入回调函数的时候，唤醒事件循环线程
    // 会有两种唤醒情况
        * 1. 会唤醒被poller_->poll(kPollTimeMs,&activeChannels_);阻塞的事件循环线程
        * 2. 事件循环线程正在执行回调函数，当他执行完后，再次调用poller_->poll(kPollTimeMs,&activeChannels_);
        * 由于有新的事件发生了（eventfd也就是wakeupChannel_有读事件）
        * 就不会被阻塞而继续执行doPendingFunctors();
    **/
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    // 存放回调函数的所有集合
    std::vector<Functor> pendingFunctors_;

    // 用来标志是否正在处理回调函数
    std::atomic_bool pcallingPendingFunctors_;

    // 用来实现共享数据的互斥访问
    std::mutex mutex_;
};

