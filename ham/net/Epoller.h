#ifndef __EpOLLER_H__
#define __EpOLLER_H__

#define OPEN_MAX 1024
#include <sys/epoll.h>
#include <boost/noncopyable.hpp>
#include <vector>

#include "net/Channel.h"
#include "base/Timestamp.h"

namespace ham
{
namespace net
{
class EventLoop;

class Epoller : public boost::noncopyable
{
public:
    Epoller(EventLoop*, int size = 1024);
    ~Epoller();

    Timestamp wait(const int kEpollTimeMs, std::vector<Channel*>& ActiveChannels);

    void updateChannel(Channel*);
    void removeChannel(Channel*);
    void fillActiveChannels(std::vector<Channel*>& ActiveChannels);

private:
    void update(int op, Channel*);

    EventLoop* loop_;
    int epoll_fd_;
    struct epoll_event retChannels_[OPEN_MAX];
    
};
}
}


#endif // __EpOLLER_H__