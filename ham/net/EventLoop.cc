#include "EventLoop.h"

namespace ham
{
    namespace net
    {
        EventLoop::EventLoop_ptr EventLoop::create() 
        {
            auto el = std::make_shared<EventLoop>();
            el->init();
            return el;
        }
        
        void EventLoop::init() 
        {
            wakeupChannel_ = std::unique_ptr<Channel>(new Channel(shared_from_this()));
        }
    }
}