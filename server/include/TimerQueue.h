#pragma once
#include <set>

#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "Channel.h"

class TimerQueue {
public:
    using TimerCallback = std::function<void()>;
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    // 增加定时器任务
    TimerId addTimer(TimerCallback cb,
                   TimeStamp when,
                   double interval);
    // 取消定时器任务
    void cancel(TimerId timerId);
private:
    using ActiverTimer = std::pair<Timer *, int64_t>;
    using ActiverTimerSet = std::set<ActiverTimer>;
    using Entry = std::pair<TimeStamp, Timer*>;
    using TimerList = std::set<Entry>;


    EventLoop* loop_;

    // timerfd的描述符
    int timerfd_;

    // 与timer描述符相对应的channel
    Channel timerfdChannel_;

    // 定时器任务列表，以触发时间排序，小的在前
    TimerList timers_;

    // 待处理的定时器列表
    ActiverTimerSet activeTimers_;

    // 标记是否正在执行要过期的定时器
    std::atomic_bool callingExpiredTimers_;

    // 暂存正要过期的定时器列表
    ActiverTimerSet cancleTimers_;

    // 取消定时器任务
    void cancelInLoop(TimerId timerid);

    // 处理readfd的读事件，得到过期的定时器，并且执行对应任务，并设置下一个要发生的定时器
    void handleRead();

    // 把过期的定时器从timers_和activeTimer_中删掉 并返回所有过期的定时器集合
    std::vector<Entry> getExpired(TimeStamp now);

    // 加入到容器中，加入时候要注意是否更新了最早触发任务
    void addTimerInLoop(Timer* timer);

    // 如果有重复的定时器就重新设置，否则就直接删除，获取过期时间最早的定时器的时间，并将它设置到timerfd中
    void reset(const std::vector<Entry>& expired, TimeStamp now);

    // 插入到ActiveTimeSet和timers，同时注意是否会影响一个要发生的定时器时间
    bool insert(Timer* timer);
};

