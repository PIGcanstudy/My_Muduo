//
// Created by ubuntu on 24-7-19.
//

#include "Acceptor.h"

#include <cassert>
#include <fcntl.h>

#include "TcpConnection.h"
#include "logger.h"
#include <iostream>


static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__,__FUNCTION__,__LINE__,errno);
    }
}

Acceptor::Acceptor(EventLoop* event_loop, const InetAddress& address, bool resuseport)
    : acceptSocket_(createNonblocking())
    , acceptChannel_(acceptSocket_.fd(),event_loop)
    , event_loop_(event_loop)
    , listening_(false)
    , idlefd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idlefd_ >= 0);
    acceptSocket_.SetReuseaddr(true);
    acceptSocket_.SetReuseport(resuseport);
    acceptSocket_.bind(address);
    setNewConnectionCallBack(std::bind(&Acceptor::handleread, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    close(idlefd_);
}

void Acceptor::listen() {
    acceptChannel_.enableReading();
    listening_ = true;
    acceptSocket_.listen(2);
}

void Acceptor::handleread() {
    InetAddress clientAddress{};
    int connfd = acceptSocket_.accept(clientAddress);
    if(connfd >= 0) {
        if(connectionCallBack_) {
            connectionCallBack_(connfd, clientAddress);
        }else {
            close(connfd);
        }
    }
    else {
        LOG_ERROR("in Acceptor::handleRead");
        // 由于采用了LT，为了防止一直通知，使用idlefd_来接受读事件
        //`idleFd_`的设计是为了优雅地处理文件描述符耗尽的情况，确保系统在高负载下仍能稳定工作。
        //通过预先打开一个文件描述符（`/dev/null`），在文件描述符耗尽时释放它来接受新连接，
        //然后再重新打开`/dev/null`，这种机制可以有效避免程序因无法分配文件描述符而崩溃。
        if (errno == EMFILE)
        {
            ::close(idlefd_);
            idlefd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idlefd_);
            idlefd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}


