#include "EventLoop.h"
#include "Channel.h"

namespace ham
{
    namespace net
    {
        Channel::Channel(std::shared_ptr<EventLoop> loop)
            : loop_(loop)
        {
            
        }
    }
}