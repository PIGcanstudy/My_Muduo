#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string &nameArg)
    : baseLoop_(baseloop)
    , started_(false)
    , name_(nameArg)
    , threadNums_(std::thread::hardware_concurrency())
    , next_(0)
{

}

EventLoopThreadPool::~EventLoopThreadPool() {
    //  因为子线程的的loop 是栈中分配的所以不需要手动删除
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    EventLoop* loop = baseLoop_;
    // 通过轮询，获取下一个处理事件的loop
    if(!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if(next_ >= loops_.size()) next_ = 0;
    }
    return loop;
}

// 空的时候是单线程
std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    if(loops_.empty()) {
        return std::vector<EventLoop*>{baseLoop_};
    }else {
        return loops_;
    }
}

void EventLoopThreadPool::start(const ThreadInitCallBack &cb) {
    started_ = true;
    for(auto i = 0; i < threadNums_; i ++) {
        char buf[128];
        snprintf(buf,sizeof buf,"%s%d",name_.c_str(),i);
        auto t = std::make_unique<EventLoopThread>(cb, buf);
        // 启动循环并创建线程
        loops_.emplace_back(t->startLoop());
        // 放入threads
        threads_.emplace_back(std::move(t));
    }

    //如果整个循环就只有一个线程，就让这个主线程来执行回调函数
    if(threadNums_ == 0 && cb) {
        cb(baseLoop_);
    }
}

