//
// Created by zxn on 24-8-23.
//

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "nocopyable.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include <string>
#include <functional>

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable
{
public:
    // 处理信息的回调函数类型
    typedef std::function<void (const HttpRequest&,
                                HttpResponse*)> HttpCallback;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name,
               TcpServer::Option option = TcpServer::kNoReusePort);

    EventLoop* getLoop() const { return server_.getLoop(); }

    // 设置回调函数
    void setHttpCallback(const HttpCallback& cb)
    {
        httpCallback_ = cb;
    }

    // 设置子事件循环线程的数量
    void setThreadNum(int numThreads)
    {
        server_.setThreadNums(numThreads);
    }

    // 启动服务
    void start();

private:
    // 连接上来的回调函数
    void onConnection(const TcpConnectionPtr& conn);

    // 有信息到达的回调函数
    void onMessage(const TcpConnectionPtr& conn,
                   Buffer* buf,
                   TimeStamp receiveTime);

    // 有请求到达的回调函数
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

    // Tcp服务器
    TcpServer server_;

    // 回调函数
    HttpCallback httpCallback_;
};



#endif //HTTPSERVER_H
