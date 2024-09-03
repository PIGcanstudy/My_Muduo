#pragma once

#include "nocopyable.h"
#include "Thread.h"
#include "EventLoop.h"

#include <mutex>
#include <condition_variable>


class EventLoopThread: public noncopyable {
public:
    using ThreadInitCallBack = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallBack& cb, const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    // 线程所执行的任务
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallBack callback_;
};


