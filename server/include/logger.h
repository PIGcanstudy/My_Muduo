#pragma once

#include <string>

#define LOG_INFO(format, ...)                           \
    do{                                                 \
        logger& log = logger::getInstance();            \
        log.setLevel(INFO);                             \
        char buf[1024];                                 \
        snprintf(buf, 1024, format, ##__VA_ARGS__);     \
        log.log(buf);                                   \
    }while(0);

#define LOG_ERROR(format, ...)                          \
    do{                                                 \
        logger& log = logger::getInstance();            \
        log.setLevel(ERROR);                            \
        char buf[1024];                                 \
        snprintf(buf, 1024, format, ##__VA_ARGS__);     \
        log.log(buf);                                   \
    }while(0);

#define LOG_FATAL(format, ...)                          \
    do{                                                 \
        logger& log = logger::getInstance();            \
        log.setLevel(FATAL);                            \
        char buf[1024];                                 \
        snprintf(buf, 1024, format, ##__VA_ARGS__);     \
        log.log(buf);                                   \
    }while(0);

// 一般关闭 通过宏打开
#ifdef MUDEBUG
#define LOG_DEBUG(format, ...)                          \
    do{                                                 \
        logger& log = logger::getInstance();            \
        log.setLevel(DEBUG);                            \
        char buf[1024];                                 \
        snprintf(buf, 1024, format, ##__VA_ARGS__);     \
        log.log(buf);                                   \
    }while(0);
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif
/*
定义日志级别
INFO:打印重要的流程信息，跟踪核心流程
ERROE：一些错误，但是不影响系统的正常运行
FATAL: 出现这种问题以后，系统不能正常的运行
DEBUG：调试信息，一般有很多，正常情况下会选择关掉，需要的时候在打开
*/

enum LogLevel {
    INFO,
    ERROR,
    FATAL,
    DEBUG
};

class logger {
public:
    ~logger() = default;
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;
    static logger& getInstance();
    // 设置日志等级
    void setLevel(int loglebel);
    // 写日志
    void log(const std::string& message) const;
private:
    int logLevel_;
    logger() = default;
};


