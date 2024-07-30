#include "Socket.h"
#include "logger.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <cstring>
#include <unistd.h>

std::unordered_map<int, std::shared_ptr<Socket>> Socket::clientsocks_;

Socket::Socket(int fd): fd_(fd)
{
    
}

Socket::Socket(int fd, InetAddress clientaddr): fd_(fd), ip_(clientaddr.ip()), port_(clientaddr.port()) {

}


Socket::~Socket()
{
    close(fd_);
}

int Socket::fd() const
{
    return fd_;
}

void Socket::SetReuseaddr(bool on)
{
    int opt = on ? 1 : 0;
    // 允许套接字绑定到一个已在使用中的本地地址和端口。例如，当服务器重启时，可以立即重新绑定到同一地址和端口，而不需要等待上一个连接完全关闭
    setsockopt(fd_,SOL_SOCKET, SO_REUSEADDR,&opt, sizeof(opt));
}

void Socket::SetReuseport(bool on)
{
    int opt = on ? 1 : 0;
    // 允许多个套接字绑定到同一个地址和端口。不同于 SO_REUSEADDR，SO_REUSEPORT 使得多个进程或线程可以共享同一个端口进行监听。
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void Socket::SetTcpNodelay(bool on)
{
   int opt = on ? 1 : 0;
    // 禁用 Nagle 算法。Nagle 算法会将小的数据包合并成一个大的数据包发送，以减少网络拥塞。而 TCP_NODELAY 选项禁用了这一算法，确保数据包立即发送。
    setsockopt(fd_, SOL_SOCKET, TCP_NODELAY, &opt, sizeof (opt));
}

void Socket::SetKeepalive(bool on)
{
    int opt = on ? 1 : 0;
    // 启用 TCP 保持连接探测机制。TCP 保持连接会周期性地发送探测消息，以确保连接仍然存活。如果探测失败，连接将被关闭。
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}

void Socket::bind(const InetAddress &servaddr)
{
    if(::bind(fd_, servaddr.addr(), sizeof servaddr) < 0){
        LOG_FATAL("bind sockfd:%d fail\n",fd_);
    }

    ip_ = servaddr.ip();
    port_ = servaddr.port();
}

void Socket::listen(int n)
{
    if(::listen(fd_, n) != 0){
        LOG_FATAL("listen sockfd:%d failed\n", fd_);
    }
}

int Socket::accept(InetAddress &clientaddr)
{
    /**
     * 1. accept函数的参数不合法
     * 2. 对返回的connfd没有设置非阻塞
     * Reactor 模型 one loop per thread
     * poller + non-blocking IO
    */

    sockaddr_in peeraddr{};

    socklen_t len = sizeof(peeraddr);

    int clientfd = accept4(fd_, (sockaddr *)&peeraddr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if(clientfd >= 0) {
        clientaddr.setaddr(peeraddr);
    }

    return clientfd;
}

std::string Socket::ip() const {
    return ip_;
}

uint16_t Socket::port() const {
    return port_;
}

int createnonblocking() // 创建监听窗口
{
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if(listenfd == -1){
        LOG_FATAL("listenfd create failed\n");
    }
    return listenfd;
}

void Socket::shutDownWrite() {
    if( shutdown(fd_, SHUT_WR) < 0) {
        LOG_ERROR("shutdownWrite error\n");
    }
}

