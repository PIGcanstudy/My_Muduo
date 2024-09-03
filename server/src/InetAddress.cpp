#include "InetAddress.h"
#include <cstring>

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    // IPV4网络协议的套接字类型
    addr_.sin_family = AF_INET;
    // 用于监听的ip地址
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    // 用于监听的端口
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const sockaddr_in& addr): addr_(addr)
{

}

InetAddress::~InetAddress()
{
}

std::string InetAddress::ip() const
{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

uint16_t InetAddress::port() const
{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t start = strlen(buf);
    uint16_t Port = port();
    memcpy(buf + start, &Port, sizeof Port);
    return buf;
}


const sockaddr *InetAddress::addr() const
{
    return (sockaddr *)&addr_;
}

InetAddress &InetAddress::operator=(const InetAddress &addr)
{
    if(this == &addr){
        return *this;
    }

    addr_ = addr.addr_;
    return *this;
}

void InetAddress::setaddr(sockaddr_in addr)
{
    addr_ = addr;
}