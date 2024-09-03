
#include "Poller.h"

#include <Channel.h>

Poller::Poller(EventLoop* loop):owernLoop_(loop) {

}

bool Poller::hasChannel(Channel* channel) {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
