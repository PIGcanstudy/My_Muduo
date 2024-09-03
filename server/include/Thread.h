#pragma once

#include "nocopyable.h"
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <functional>
class Thread:public noncopyable{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_;}

    static int numCreated() { return numCreated_;}

private:
    void setDefaultName();

    std::string name_;
    bool started_;
    bool joined_;
    pid_t tid_;
    std::shared_ptr<std::thread> thread_;

    // 线程要执行的任务
    ThreadFunc func_;

    // 记录线程开辟了多少用于编号
    static std::atomic_int numCreated_;
};

