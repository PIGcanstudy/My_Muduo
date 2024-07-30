#pragma once

#include "Socket.h"
#include <memory>
#include <functional>

/*
理清楚 EventLoop Channel，Poller之间的关系 他们在Reactor上面对应的Demultiplex
Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN  EPOLLOUT事件
还绑定了poller返回的具体事件
*/
class EventLoop;
class TimeStamp;

// 一个监听端口和一个epoll即为一个channel
class Channel{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;

    Channel(int fd, EventLoop* event_loop);
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    ~Channel() = default;
    //返回fd_
    int fd() const;

    // 采用边缘触发
    void usetET();

    // 设置inepoll成员为true
    void setinepoll();

    // 设置revents成员的值为参数ev
    void setrevents(uint32_t ev);

    // 返回 eventsl成员
    uint32_t events() const;

    // 返回 inepoll成员
    bool inepoll() const;

    // 返回 revents成员
    uint32_t revents() const;

    // 处理epoll_wait返回的事件
    void handleevent(TimeStamp receiveTime) const;

    // 防止当channel被手动remove掉，channel还在执行回调操作时失效
    void tie(const std::shared_ptr<void>& obj);

    // 设置fd相应的状态 update()相当于调用epoll_ctl
    void enableReading() { events_ |= kReadEvent; update();} //相当于把读事件给events相应的位置位了
    void disableReading() { events_ &= ~kReadEvent; update();}
    void enableWriting() { events_ |= kWriteEvent; update();}
    void disableWriting() { events_ &= ~kWriteEvent; update();}
    void disableAll() { events_ = kNoneEvent; update();}

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent;}
    bool isReadEvent() const { return events_ & kReadEvent;}
    bool isWriteEvent() const { return events_ & kWriteEvent;}

    // one loop per thread
    EventLoop* onwerLoop() {return event_loop_;}

    void remove();

    // 设置写事件回调函数
    void setWriteCallBack(EventCallback wcb);

    // 设置读事件回调函数
    void setReadCallBack(ReadEventCallback rcb);

    // 设置连接断开回调函数
    void setCloseCallBack(EventCallback rcb);

    // 设置连接错误回调函数
    void setErrorCallBack(EventCallback rcb);

private:

    // 表示没有感兴趣的事件
    static const int kNoneEvent;
    // 表示感兴趣的是读事件
    static const int kReadEvent;
    // 表示感兴趣的是写事件
    static const int kWriteEvent;

    void update();

    void handleEventWithGuard(TimeStamp receiveTime) const;

    // channel拥有的fd，Channel和fd是一一对应的关系
    int fd_;

    // channel对应的红黑树，channel与EpollLoop是多对一的关系，一个Channel只对应一个EpollLoop
    // 一个EpollLoop可以对应多个Channel
    EventLoop* event_loop_;

    // Channel是已经添加到对应的epoll树中，false为未添加，true表示已经添加
    // 如果已经添加用EPOLL_CTL_MOD 否则用 EPOLL_CTL_ADD
    bool inepoll_;

    // 当客户端正常断开TCP连接，IO事件会触发Channel中的设置的CloseCallback回调
    // 但是用户代码在onClose()中有可能析构Channel对象，导致回调执行到一半的时候，其所属的Channel对象本身被销毁了
    // 为了解决这个问题 考虑延长生命周期，怎么延长？
    // 如果直接在另一个类声明一个强引用，但是这会出现循环引用问题
    // 所以想着使用弱引用，那弱引用该如何延长生命周期呢?
    // 可以在调用函数之前 将它提升为强引用赋值给一个强引用 从而增加引用计数
    // 在调用完某函数之前都不会引用计数变为0，执行完后出作用域，引用计数-1

    std::weak_ptr<void> tie_; // 一方面这个若引用可以做到避免循环引用的现象，另一方面可以增加引用计数

    // 表示当前的 Channel对象 是否和一个生命周期受控的对象（如 `TcpServer` 或 `EventLoop`）关联。
    // 如果 `tied_` 为 `true`，则表示需要检查关联对象的生命周期。
    bool tied_;


    // fd需要监听的事件，listenfd和clientfd需要监听EPOLLIN，
    // clientfd还可能监听EPOLLOUT事件
    uint32_t events_;
    // fd_中已发生的事件
    uint32_t revents_;


    // 因为channel可以获得fd最终发生的具体事件revent，所以他负责回调
    // 读事件回调函数
    ReadEventCallback readcallback_;
    // 连接错误回调函数
    EventCallback errorcallback_;
    // 连接断开回调函数
    EventCallback closecallback_;
    // 写事件回调函数
    EventCallback writecallback_;

};
