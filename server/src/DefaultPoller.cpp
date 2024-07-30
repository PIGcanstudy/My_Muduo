#include "Poller.h"
#include "EPollPoller.h"
#include <cstdlib>

std::unique_ptr<Poller> Poller::newDefaultPoller(EventLoop* loop) {
    if(::getenv("MODUO_USE_POLL"))
    {
        return nullptr; //生成poll的实例
    }
    else
    {
        return std::make_unique<EPollPoller>(loop); //生成epoll的实例
    }
}
