#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <cstring>
#include <sys/types.h>
#include <memory>
#include "include/TimeStamp.h"
#include "include/InetAddress.h"
#include "include/TcpConnection.h"
#include "include/EventLoop.h"
#include "include/TcpServer.h"
#include "include/logger.h"

class EchoServer {
public:
    EchoServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg)
        : loop_(loop)
        , tcpServer_(loop, listenAddr, nameArg)
    {
        Start();
        //注册回调函数
        tcpServer_.setConnectionCallback(
            std::bind(&EchoServer::onConnection,this,std::placeholders::_1)
        );

        tcpServer_.setMessageCallback(
            std::bind(&EchoServer::onMessage,this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );

        //设置合适的loop线程数量 loopThread
        tcpServer_.setThreadNums(std::thread::hardware_concurrency());
    }

    void Start() {
        tcpServer_.start();
    }
private:

    //可读写事件回调
    void onMessage(const TcpConnectionPtr &conn,
                Buffer *buf,
                TimeStamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        LOG_INFO("%s recv %d bytes at ", conn->name(), msg.size(),time.to_string());
        if (msg == "exit\n")
        {
            conn->send("bye\n");
            conn->shutdown();
        }
        if (msg == "quit\n")
        {
            loop_->quit();
        }
        conn->send(msg);
    }

    //连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            LOG_INFO("Connection UP : %s",conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN : %s",conn->peerAddress().toIpPort().c_str());
        }
    }

    EventLoop* loop_;
    TcpServer tcpServer_;
};

int main(int argc, char* argv[]){
    EventLoop loop;
    InetAddress addr(8000);
    //Acceptor non-blocking listenfd create bind
    EchoServer server(&loop,addr,"EchoServer-01");
    //listen loopthread listenfd => acceptChannel => mainLoop => subloop
    server.Start();
    loop.loop(); //启动mainloop的底层pooler
    return 0;
}
