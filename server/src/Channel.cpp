#include <iostream>
#include <TcpServer.h>

#include "EPollPoller.h"
#include "Channel.h"
#include "InetAddress.h"
#include "logger.h"
#include "TimeStamp.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel(int fd, EventLoop* event_loop)
        : fd_(fd)
        , event_loop_(event_loop)
        , inepoll_(false)
        , tied_(false)
        , events_(0)
        , revents_(0)
{

}

int Channel::fd() const
{
    return fd_;
}

void Channel::usetET()
{
    events_ = events_|EPOLLET;
}

void Channel::setinepoll()
{
    inepoll_ = true;
}

void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
}

uint32_t Channel::events() const
{
    return events_;
}

bool Channel::inepoll() const
{
    return inepoll_;
}

uint32_t Channel::revents() const
{
    return revents_;
}

void Channel::setReadCallBack(ReadEventCallback rcb){
    readcallback_ = std::move(rcb);
}

void Channel::setCloseCallBack(EventCallback rcb) {
    closecallback_ = std::move(rcb);
}

void Channel::setErrorCallBack(EventCallback rcb) {
    errorcallback_ = std::move(rcb);
}

void Channel::setWriteCallBack(EventCallback wcb) {
    writecallback_ = std::move(wcb);
}

void Channel::handleevent(TimeStamp receiveTime) const {
    if(tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if(guard) {
            handleEventWithGuard(receiveTime);
        }
    }else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(TimeStamp receiveTime) const{
    LOG_INFO("channel handleEvent revents:%d\n",revents_);

    // EPOLLRDHUP 表示对方已经关闭
    if((revents_ & EPOLLRDHUP) && !(revents_ & EPOLLIN)){
        if (closecallback_) {
            closecallback_();
        }
        return;
    }
    // EPOLLPRI 表示外带数据
    if(revents_ & (EPOLLIN | EPOLLPRI) ) {
       if (readcallback_) {
            readcallback_(receiveTime);
        } else {
            LOG_ERROR("Read callback not set for fd= %d\n", fd_);
        }
    }
    else if(revents_ & EPOLLOUT)
    {
        if(writecallback_)
        {
            writecallback_();
        }
    }
    else
    {
        if(errorcallback_) errorcallback_();
    }
}

void Channel::update() {

    //通过channel所属的EventLoop，把当前的channel删除掉
    event_loop_->updatechannel(this);
}

void Channel::remove() {
    //通过channel所属的EventLoop，调用Poller相应的方法，移除fd的events事件
    event_loop_->removechannel(this);
}

void Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

