#pragma once
#include <cstdint>


class Timer;

class TimerId
{
public:
    TimerId()
      : timer_(NULL),
        sequence_(0)
    {
    }

    TimerId(Timer* timer, int64_t seq)
      : timer_(timer),
        sequence_(seq)
    {
    }

    TimerId(const TimerId& timerid) = default;

    TimerId& operator=(const TimerId& timerid) = default;

    friend class TimerQueue;

private:
    // 指向一个定时器
    Timer* timer_;

    // 该定时器对应编号
    int64_t sequence_;
};
