//
// Created by ubuntu on 24-7-24.
//

#include "../include/TimeStamp.h"

#include <chrono>

TimeStamp::TimeStamp():microSecondsSinceEpoch_(0) {

}

TimeStamp::TimeStamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch) {

}

TimeStamp TimeStamp::now() {
    return TimeStamp (time(NULL));
}

std::string TimeStamp::to_string() const {
    char buf[128]{};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
    tm_time->tm_year+1900,
    tm_time->tm_mon+1,
    tm_time->tm_mday,
    tm_time->tm_hour,
    tm_time->tm_min,
    tm_time->tm_sec);
    return buf;
}

TimeStamp TimeStamp::invaild() {
    return TimeStamp{};
}

TimeStamp TimeStamp::addTime(TimeStamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * TimeStamp::kMicroSecondsPerSecond);
    return TimeStamp(timestamp.microSecondsSinceEpoch() + delta);
}




