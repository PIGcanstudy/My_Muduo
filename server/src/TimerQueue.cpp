#include "TimerQueue.h"
#include "logger.h"
#include <sys/timerfd.h>

timespec howMuchTimeFromNow(TimeStamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch()
                         - TimeStamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / TimeStamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % TimeStamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

int createTimerfd() {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0) {
        LOG_ERROR("timerfd_create failed!");
    }
    return timerfd;
}

void resetTimerfd(int timerfd, TimeStamp expiration) {
    itimerspec newValue{};
    itimerspec oldValue{};
    // 定时的间隔时间
    newValue.it_value =  howMuchTimeFromNow(expiration);
    int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if(ret < 0) {
        LOG_ERROR("timerfd_settime failed");
    }
}

void readTimerfd(int timerfd, TimeStamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_INFO("TimerQueue::handleRead() %lu at %s", howmany, now.to_string().c_str());
    if (n != sizeof howmany)
    {
        LOG_ERROR("TimerQueue::handleRead() reads %ld bytes instead of 8", n);
    }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(timerfd_, loop)
    , timers_()
    , activeTimers_()
    , callingExpiredTimers_(false)
    , cancleTimers_()
{
    timerfdChannel_.setReadCallBack(std::bind(&TimerQueue::handleRead, this));

    // 设置可读 并且让loop监听读事件
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    close(timerfd_);
    for(auto& t: timers_) {
        delete t.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, TimeStamp when, double interval) {
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(std::bind(&TimerQueue::addTimerInLoop, this, timer)));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer) {
    bool earliestChanged = insert(timer);
    if(earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

bool TimerQueue::insert(Timer *timer) {
    bool earliestChanged = false;
    TimeStamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if(it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator, bool> result
          = timers_.insert(Entry(when, timer));
    }

    {
        std::pair<ActiverTimerSet::iterator, bool> result
          = activeTimers_.insert(ActiverTimer(timer, timer->sequence()));
    }

    return earliestChanged;

}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
      std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerid) {
    ActiverTimer atimer(timerid.timer_, timerid.sequence_);
    ActiverTimerSet::iterator it = activeTimers_.find(atimer);
    if (it != activeTimers_.end())
    {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        (void)n;
        delete it->first; // FIXME: no delete please
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_)
    {
        cancleTimers_.insert(atimer);
    }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now) {
    std::vector<TimerQueue::Entry> expired;
    // 这是一个哨兵
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    // 找到第一个大于或等于的定时器
    auto end = timers_.lower_bound(sentry);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry& it : expired) {
        ActiverTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
    }
    return std::move(expired);
}

void TimerQueue::handleRead() {
    TimeStamp now(TimeStamp::now());

    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancleTimers_.clear();
    for (const Entry& it : expired)
    {
        it.second->run();
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
}

void TimerQueue::reset(const std::vector<Entry> &expired, TimeStamp now) {
    TimeStamp nextExpire;

    for (const Entry& it : expired) {
        ActiverTimer timer(it.second, it.second->sequence());
        if (it.second->repeat()
            && cancleTimers_.find(timer) == cancleTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            delete it.second;
        }
    }

    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}








