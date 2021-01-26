#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__
#include <memory>
#include "Channel.h"

namespace ham
{
namespace net
{

class EventLoop : public std::enable_shared_from_this<EventLoop>
{
public:
    typedef std::shared_ptr<EventLoop> EventLoop_ptr;

    static EventLoop_ptr create();
    void init();
private:
    std::unique_ptr<Channel> wakeupChannel_;
    //std::shared_ptr<Channel> currentActiveChannel_;
};
}
}

#endif // __EVENTLOOP_H__