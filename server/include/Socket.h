#pragma once
#include <unordered_map>
#include <memory>
#include "InetAddress.h"

int createnonblocking();

class Socket{
public:
    Socket(int fd);
    Socket(int fd, InetAddress clienaddr);
    ~Socket();

    int fd() const; // 返回fd_成员

    std::string ip() const;

    uint16_t port() const;

    void bind(const InetAddress& servaddr);
    void listen(int n = 128);
    int accept(InetAddress& clientaddr);

    // 允许套接字绑定到一个已在使用中的本地地址和端口。例如，当服务器重启时，可以立即重新绑定到同一地址和端口，而不需要等待上一个连接完全关闭
    void SetReuseaddr(bool on);
    // 允许多个套接字绑定到同一个地址和端口。不同于 SO_REUSEADDR，SO_REUSEPORT 使得多个进程或线程可以共享同一个端口进行监听。
    void SetReuseport(bool on);
    // 禁用 Nagle 算法。Nagle 算法会将小的数据包合并成一个大的数据包发送，以减少网络拥塞。而 TCP_NODELAY 选项禁用了这一算法，确保数据包立即发送。
    void SetTcpNodelay(bool on);
    // 启用 TCP 保持连接探测机制。TCP 保持连接会周期性地发送探测消息，以确保连接仍然存活。如果探测失败，连接将被关闭。
    void SetKeepalive(bool on);

    // 用来关闭写事件
    void shutDownWrite();

    // 用来存储练上来的连接
    static std::unordered_map<int, std::shared_ptr<Socket>> clientsocks_;

private:

    const int fd_;
    std::string ip_{""};
    uint16_t port_{0};
};