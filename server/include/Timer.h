#pragma once
#include <atomic>
#include <functional>
#include "TimeStamp.h"

#include "nocopyable.h"

class Timer: public noncopyable{
public:
    using TimerCallback = std::function<void()>;
    Timer(TimerCallback timercallback, TimeStamp when, double interval);
    ~Timer() = default;

    TimeStamp expiration() const  { return expiration_; }

    bool repeat() const { return repeat_; }

    int64_t sequence() const { return sequence_; }

    void run() const{
        timerCaLLBack_();
    }

    void restart(TimeStamp now);

    static int64_t numCreated() { return numCreated_; }
private:

    // 定时到期需要执行的任务
    TimerCallback timerCaLLBack_;

    // 何时到期
    TimeStamp expiration_;

    // 距离下次启动的时间间隔，如果不是重复定时器，其值为0
    double interval_;

    // 标志是否是重复定时器
    bool repeat_;

    // 定时器的唯一ID
    std::int64_t sequence_;

    // 定时器开辟的数量
    static std::atomic_int64_t numCreated_;
};


