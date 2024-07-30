#pragma once

#include <cstdint>
#include <string>

class TimeStamp {
public:
    TimeStamp();
    explicit TimeStamp(int64_t microSecondsSinceEpoch);
    ~TimeStamp() = default;
    static TimeStamp now();
    std::string to_string() const;
private:
    int64_t microSecondsSinceEpoch_;
};


