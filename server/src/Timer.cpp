#include "Timer.h"
#include "TimeStamp.h"

std::atomic_int64_t Timer::numCreated_ = 0;

Timer::Timer(TimerCallback timercallback, TimeStamp when, double interval)
    : timerCaLLBack_(timercallback)
    , expiration_(when)
    , interval_(interval)
    , repeat_(interval > 0.0)
    , sequence_(++ numCreated_)
{

}

void Timer::restart(TimeStamp now) {
    if (repeat_)
    {
        expiration_ = TimeStamp::addTime(now, interval_);
    }
    else
    {
        expiration_ = TimeStamp::invaild();
    }
}
