#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

// socket的地址协议类
class InetAddress{
public:
    // 用来构造监听的fd
    InetAddress(uint16_t port = 0, std::string ip="127.0.0.1");
    // 用来构造客户端连上来的fd
    InetAddress(const sockaddr_in& addr);
    //
    InetAddress& operator=(const InetAddress& addr);

    ~InetAddress();

    //返回字符串表示的地址
    std::string ip() const;

    std::string toIpPort() const;

    // 返回整数表示的端口
    uint16_t port() const;

    // 返回addr_成员地址，因为要用来绑定所以要转换为sockaddr
    const sockaddr* addr() const;

    void setaddr(sockaddr_in addr);
private:
    sockaddr_in addr_;

};