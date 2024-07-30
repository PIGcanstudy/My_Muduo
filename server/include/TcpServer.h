#pragma once
#include "EventLoop.h"
#include "callbacks.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
class Connection;

class TcpServer {
public:
    //提供了一个注册函数
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    enum Option //是否对端口可重用
   {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
            const InetAddress &listenAddr,
            const std::string  &nameArg,
            Option option = kNoReusePort);

    ~TcpServer();

    // 下面的回调都是用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb;}
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) {writeCompleteCallback_ = cb;}

    //开始服务器监听
    void start();

    // 设置subloop的个数
    void setThreadNums(const int threadNums = std::thread::hardware_concurrency());
private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    //baseloop_用户定义的loop
    EventLoop *loop_;

    // 本地ip端口
    const std::string ipPort_;

    // 名称
    std::string name_;

    // 用来连接新到的客户端
    std::unique_ptr<Acceptor> acceptor_;

    // 指向一个线程池
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    // 有新连接时的回调
    ConnectionCallback connectionCallback_;

    // 有读写消息时的回调
    MessageCallback messageCallback_;

    //消息发送完成以后的回调
    WriteCompleteCallback writeCompleteCallback_;

    //LOOP线程初始化的回调 std::function类型 调用者，调用回调函数
    ThreadInitCallback threadInitCallback_;

    //防止一个TcpServer对象被start多次
    std::atomic_int started_;

    int nextConnId_;

    // 用来存储Connection的map
    ConnectionMap connMap_;
};
