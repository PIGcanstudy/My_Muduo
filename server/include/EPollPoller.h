#pragma once

#include "Poller.h"
#include <sys/epoll.h>
#include <vector>
#include <memory>

/*
epoll的使用
epoll_create    也就是EPollPoller
epoll_ctl  add/mol/del   也就是updateChannel removeChannel
epoll_wait   也就是poll
*/

class Channel;
class EPollPoller : public Poller{
public:
    // 创建epollfd
    explicit EPollPoller(EventLoop* loop);
    // 关闭epollfd
    ~EPollPoller() override;

    // 重写基类Poller的抽象方法
    TimeStamp poll(int timeoutMs, ChannelList &activeChannels) override;

    // 把channel添加或者更新到epoll的红黑树，channel有fd，也有需要监视的事件
    void updateChannel(Channel* channel) override;

    void removeChannel(Channel* channel) override;

private:
    using EventList = std::vector<epoll_event>;

    //epoll_event初始的长度
    static constexpr int kInitEventListSize = 100;

    //填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList &activeChannels) const;

    // 根据opeation 判断对epoll底层红黑树的操作
    void update(int operation, Channel* channel) const;


    //epoll 的句柄
    int epollfd_;

    // 存放epoll返回的事件的容器
    EventList events_;

};
