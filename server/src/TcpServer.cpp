#include "TcpServer.h"

#include <strings.h>
#include <TcpConnection.h>

#include "logger.h"

static EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d mainloop is null! \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
    : loop_(CheckLoopNotNull(loop))
    , ipPort_(listenAddr.toIpPort())
    , name_(nameArg)
    , acceptor_(std::make_unique<Acceptor>(loop, listenAddr, option == kReusePort))
    , threadPool_(std::make_shared<EventLoopThreadPool>(loop, nameArg))
    , messageCallback_()
    , writeCompleteCallback_()
    , started_(0)
    , nextConnId_(0)
    , connMap_()
{
    acceptor_->setNewConnectionCallBack(std::bind(&TcpServer::newConnection, this,
        std::placeholders::_1, std::placeholders::_2));
}

// 遍历connMap_, 挨个删除map中的内容，并将TcpConnection::connectDestroyed投递到conn的loop中
TcpServer::~TcpServer() {
    for(auto& item: connMap_) {
        //这个局部的shared_ptr智能指针对象，出右括号
        //可以自动释放new出来的TcpConnetion对象资源
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        //销毁连接
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn)
        );
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    // 1.先用轮询算法选出一个subloop
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf,"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
                name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

    //2.通过socket获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local,sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd,(sockaddr*) &local,&addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }

    InetAddress localAddr(local);

    //3.根据连接成功的sockfd，创建 TcpConnection连接对象conn
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(
        ioLoop, connName, sockfd, localAddr, peerAddr);

    //4.下面的回调都是用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
    connMap_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    //5.设置了如何关闭连接的回调 conn->shutdown
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection,this,std::placeholders::_1)
    );

    //6.直接调用TcpConnection::connectEstablished
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}

// 启动线程池，并把Acceptor监听加到事件循环中
void TcpServer::start() {
    //防止一个TcpServer对象被start多次
    if(started_++ == 0) {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

// 设置threadPool_的线程数量
void TcpServer::setThreadNums(const int threadNums) {
    threadPool_->setThreadNums(threadNums);
}

// 将删除操作投递到事件循环中
void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

// 先将connMap里的删掉，在调用TcpConnection的删除函数
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection \n",
    name_.c_str(),conn->name().c_str());

    connMap_.erase(conn->name()); //从map表中删除
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed,conn)
    );
}






