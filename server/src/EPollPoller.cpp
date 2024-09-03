#include <iostream>
#include <memory>
#include <unistd.h>
#include <unordered_map>

#include "EPollPoller.h"
#include "logger.h"
#include "TimeStamp.h"
#include "Channel.h"

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop)
    , epollfd_(epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if (epollfd_ == -1) {
       LOG_FATAL("epoll_create error:%d \n", errno);
    }
   LOG_INFO("epoll_create() successed");
}

EPollPoller::~EPollPoller()
{
    close(epollfd_);
}

//通过epoll_wait将发生事件的channel通过activeChannels告知给EventLoop
TimeStamp EPollPoller::poll(int timeoutMs, ChannelList &activeChannels) {

    LOG_INFO("func=%s => fd total count:%lu \n",__FUNCTION__, channels_.size());
    //events_是vector类型，
    //events_.begin()返回首元素的地址，
    //*events_.begin()为首元素的值
    //&*events_.begin()存放首元素的地址
    //这就得到了vector底层首元素的起始地址
    int EventNums = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    //记录最开始poll里面的错误值
    int saveErrno = errno;

    // 获取发生事件的时间
    TimeStamp now(TimeStamp::now());

    if(EventNums > 0) {
        LOG_INFO("%d events happended!\n", EventNums);
        fillActiveChannels(EventNums, activeChannels);
        // 为了防止频繁开辟空间，选择提前开辟
        if(EventNums == events_.size())
        {
            events_.resize(events_.size() * 2);
            //说明当前发生的事件可能多于vector能存放的 ，需要扩容，等待下一轮处理
        }
    }
    else if(EventNums == 0) {
        LOG_DEBUG("%s timeout! \n",__FUNCTION__);
    }
    else {
        if(saveErrno != EINTR) //不是外部中断引起的
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() errno!");
        }
    }

    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList &activeChannels) const {
    for(int i = 0; i < numEvents; i ++) {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        // 设置channel的返回事件
        channel->setrevents(events_[i].events);
        //EventLoop就拿到了他的poller给他返回的所有发生事件的channel列表了
        activeChannels.push_back(channel);
    }
}

//channel update remove => EventLoop updateChannel removeChannel =>Poller updateChannel removeChannel
/**
 *             EventLoop => poller.poll
 *   ChannelList          Poller
 *                     ChannelMap <fd,channel*>   epollfd
*/

void EPollPoller::updateChannel(Channel* channel) {
    LOG_INFO("func=%s => fd=%d  events=%d\n",__FUNCTION__, channel->fd(),channel->events());
    // false 表示不再epoll的红黑树中
    if(!channel->inepoll()) {
        update(EPOLL_CTL_ADD, channel);
        channels_[channel->fd()] = channel;
    }else {
        if(channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channels_.erase(channel->fd());
        }else {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}


void EPollPoller::removeChannel(Channel* channel) {
    if(!channel->inepoll()) {
        LOG_ERROR("channel not in epoll");
    }else {
        channels_.erase(channel->fd());
        LOG_INFO("func=%s => fd=%d  \n",__FUNCTION__, channel->fd());
        update(EPOLL_CTL_DEL, channel);
    }
}

// operation分别是epoll_(ADD/MOD/DEL)
void EPollPoller::update(int operation, Channel* channel) const {

    epoll_event event{0};
    event.data.ptr = channel;
    // 把感兴趣的事件加入到events中
    event.events = channel->events();
    event.data.fd = channel->fd();

    if(epoll_ctl(epollfd_, operation, channel->fd(), &event) < 0) {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n",errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n",errno);
        }
    }
}


