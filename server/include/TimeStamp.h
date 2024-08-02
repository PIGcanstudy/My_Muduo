#pragma once

#include <cstdint>
#include <string>

class TimeStamp {
public:
    // 秒转换为微秒的进位
    static const int kMicroSecondsPerSecond = 1000 * 1000;
    TimeStamp();
    explicit TimeStamp(int64_t microSecondsSinceEpoch);
    ~TimeStamp() = default;
    static TimeStamp now();
    static TimeStamp addTime(TimeStamp timestamp, double seconds);
    static TimeStamp invaild();
    std::string to_string() const;
    int64_t microSecondsSinceEpoch() const {return microSecondsSinceEpoch_;}
    bool valid() const { return microSecondsSinceEpoch_ > 0; }
private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}


