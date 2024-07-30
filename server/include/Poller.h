#pragma once

/*
muduo库中 多路事件分发器的核心IO复用模块
*/
#include <unordered_map>
#include <memory>
#include <vector>

#include "nocopyable.h"
class TimeStamp;
class Channel;
class EventLoop;

class Poller: noncopyable {
public:
    using ChannelList = std::vector<Channel*>;
    explicit Poller(EventLoop* loop);
    virtual ~Poller() = default;

    // 给所有IO复用保留的统一接口
    virtual TimeStamp poll(int timeoutMs, ChannelList& activeChannel) = 0;
    // 更新channel
    virtual void updateChannel(Channel* channel) = 0;
    // 删除channel
    virtual void removeChannel(Channel* channel) = 0;

    // 判断channel是否在当前的Poller中
    bool hasChannel(Channel* channel);

    //EventLoop可以通过改接口获取默认的IO复用的具体实现
    /**
   * 它的实现并不在 Poller.cc 文件中
   * 因为 Poller 是一个基类。
   * 如果在 Poller.cc 文件内实现则势必会在 Poller.cc包含 EPollPoller.h PollPoller.h等头文件。
   * 那么外面就会在基类引用派生类的头文件，这个抽象的设计就不好
   * 所以外面会单独创建一个 DefaultPoller.cc 的文件去实现
   */
    static std::unique_ptr<Poller> newDefaultPoller(EventLoop* loop);

protected:
    //map的key表示 sockfd  value表示所属的channel通道类型
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    // 定义Poller所属的事件循环EventLoop
    EventLoop* owernLoop_;
};

