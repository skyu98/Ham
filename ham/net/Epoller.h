#ifndef __EpOLLER_H__
#define __EpOLLER_H__

#include <sys/epoll.h>
#include <boost/noncopyable.hpp>
#include <vector>
#include <map>
#include <string>

#include "base/Timestamp.h"

namespace ham
{
namespace net
{
class EventLoop;
class Channel;

class Epoller : public boost::noncopyable
{
public:
    Epoller(EventLoop*);
    ~Epoller();

    Timestamp wait(const int kEpollTimeMs, std::vector<Channel*>& ActiveChannels);

    void updateChannel(Channel*);
    void removeChannel(Channel*);
    void fillActiveChannels(int numOfEvents, std::vector<Channel*>& ActiveChannels) const;
    
private:
    static std::string operationToString(int op);
    void update(int op, Channel*);

    EventLoop* loop_;
    int epoll_fd_;
    static const int kInitEventListSize = 16;
    std::vector<struct epoll_event> retEpoll_event_;
    std::map<int, Channel*> channelMap_;    // use for debug mode
};
}
}


#endif // __EpOLLER_H__