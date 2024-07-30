#include "logger.h"
#include "TimeStamp.h"
#include <iostream>

logger &logger::getInstance() {
    static logger instance{};
    return instance;
}

void logger::setLevel(int loglebel) {
    logLevel_ = loglebel;
}

void logger::log(const std::string& message) const{
    switch (logLevel_)
    {
        case INFO:
            std::cout<<"[INFO]";
        break;
        case ERROR:
            std::cout<<"[ERROR]";
        break;
        case FATAL:
            std::cout<<"[FATAL]";
        break;
        case DEBUG:
            std::cout<<"[DEBUG]";
        break;
        default:
            break;
    }

    //打印时间和msg
    std::cout<< TimeStamp::now().to_string() <<" : "<< message <<std::endl;
}

