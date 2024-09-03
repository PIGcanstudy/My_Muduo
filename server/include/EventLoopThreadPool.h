#pragma once
#include "EventLoop.h"
#include "EventLoopThread.h"


class EventLoopThreadPool {
public:
    using ThreadInitCallBack = std::function<void(EventLoop*)>;
    explicit EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg = std::string{});
    ~EventLoopThreadPool();

    // 设置线程池子的线程数量
    void setThreadNums(const signed int threadNums) { threadNums_ = threadNums;}

    // 开启事件循环线程
    void start(const ThreadInitCallBack &cb = ThreadInitCallBack());

    //若是多线程，baseLoop_默认以轮询的方式分配channel给subloop
    EventLoop* getNextLoop();

    // 得到所有的EventLoop
    std::vector<EventLoop*> getAllLoops();

    bool started() const{ return started_;}

    const std::string name() const { return name_;}
private:
    // 指向主EventLoop，专门用来接受新链接
    EventLoop* baseLoop_;

    // 标志池子启动
    bool started_;
    std::string name_;

    // 子线程的数量
    unsigned int threadNums_;

    // 以轮询的方式将新连接交给subLoop 此为下表索引
    int next_;

    // 存储事件循环线程的容器
    std::vector<std::unique_ptr<EventLoopThread>> threads_;

    // 存储事件循环线程中对应的EventLoop的容器
    std::vector<EventLoop *> loops_;
};

