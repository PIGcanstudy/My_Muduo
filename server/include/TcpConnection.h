#pragma once

#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "nocopyable.h"
#include "callbacks.h"

/**
 * TcpServer 通过Acceptor 有一个新用户连接，通过accept函数拿到connfd
 * 打包TcpConnection 设置回调 => channel =>poller => channel的回调
 *
*/

constexpr int MSG_HEAD_LEN = 4;

class TcpConnection: noncopyable,public std::enable_shared_from_this<TcpConnection>{
public:
    TcpConnection(EventLoop *loop,
            const std::string &name,
            int sockfd,
            const InetAddress &localAddr,
            const InetAddress &peerAddr);
    ~TcpConnection();

    const std::string& name() const { return name_;}
    const InetAddress& localAddress() const { return localAddr_;}
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected;}
    //发送数据
    void send(const std::string &buf);

    //关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_= highWaterMark;
    }

    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }

    //建立连接
    void connectEstablished();

    //销毁连接
    void connectDestroyed();

    EventLoop* getLoop() const{ return loop_;}

private:

    //已经断开连接，正在连接，已经连接，正在断开连接
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};
    void setState(StateE state) { state_ = state; }

    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    /**
   * 发送数据，应用写得快，内核发送数据慢，
   * 需要把待发送的数据写入缓冲区
   * 且设置了水位回调，防止发送太快
   */
    void sendInLoop(const void* message, size_t len);

    void shutdownInLoop();



    // 一个subloop
    EventLoop *loop_;

    // Connection的名字
    const std::string name_;

    //Connection的状态
    std::atomic_int state_;

    // 是否正在读
    bool reading_;

    // Connection对应的socket
    std::unique_ptr<Socket> socket_;

    // Connection对应的channel
    std::unique_ptr<Channel> channel_;

    // 本地的地址
    InetAddress localAddr_;

    // 对端的地址
    InetAddress peerAddr_;

    // 以下三个回调都是用户设置给TcpServer=>（传给）TcpConnection=>Channel=>Poller=>notify channel调用回调
    ConnectionCallback connectionCallback_; //有新连接时的回调
    MessageCallback messageCallback_; //有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; //消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_; // 高水位回调


    CloseCallback closeCallback_; // 关闭回调


    // 水位的阀值
    size_t highWaterMark_;

    // 接受数据的缓冲区
    Buffer inputBuffer_;

    // 发送数据的缓冲区
    Buffer outputBuffer_;
};
