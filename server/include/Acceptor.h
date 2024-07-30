#pragma once

#include <functional>
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"

class Acceptor {
public:
    using NewConnectionCallBack = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* event_loop, const InetAddress& address, bool resuseport);
    ~Acceptor();
    void listen();
    void setNewConnectionCallBack(const NewConnectionCallBack& cb) { connectionCallBack_ = cb;}
private:
    void handleread();
    Socket acceptSocket_;
    Channel acceptChannel_;
    EventLoop* event_loop_;
    NewConnectionCallBack connectionCallBack_;
    bool listening_;
    int idlefd_;
};

