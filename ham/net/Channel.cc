#include "net/EventLoop.h"
#include "base/Log.h"
#include "net/Channel.h"
#include <sstream>

namespace ham
{
    namespace net
    {
        Channel::Channel(EventLoop* loop, int fd)
            : loop_(loop),
              fd_(fd),
              event_(0),
              revent_(0),
              status_(status::kNew),
              isHandlingEvent_(false)
        {
            
        }

        Channel::~Channel()
        {
            assert(!isHandlingEvent_);
        }

        
        void Channel::update() 
        {
            loop_->updateChannel(this);
        }
        
        std::string Channel::eventsToString(int fd, int event) 
        {
            std::ostringstream oss;
            oss << fd << ": ";
            if (event & EPOLLIN)
                oss << "IN ";
            if (event & EPOLLPRI)
                oss << "PRI ";
            if (event & EPOLLOUT)
                oss << "OUT ";
            if (event & EPOLLHUP)
                oss << "HUP ";
            if (event & EPOLLRDHUP)
                oss << "RDHUP ";
            if (event & EPOLLERR)
                oss << "ERR ";
            return oss.str();
        }
        
        /*
        EPOLLIN	    普通或优先级带数据可读
        EPOLLRDNORM	普通数据可读
        EPOLLRDBAND	优先级带数据可读
        EPOLLPRI	高优先级数据可读
        EPOLLOUT	普通数据可写
        EPOLLWRNORM	普通数据可写
        EPOLLWRBAND	优先级带数据可写
        EPOLLERR	发生错误
        EPOLLHUP	对方描述符挂起
        */
        void Channel::handleEventWithGuard(Timestamp receiveTime) 
        {
            isHandlingEvent_ = true;
            if ((revent_ & EPOLLHUP) && !(revent_ & EPOLLIN))
            {
                // if (logHup_)
                // {
                // LOG_WARN << "Channel::handle_event() EPOLLHUP";
                // }
                if (closeCb_) closeCb_();
            }
            if (revent_ & EPOLLERR)
            {
                if (errorCb_) errorCb_();
            }
            if (revent_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
            {
                if (readCb_) readCb_(receiveTime);
            }
            if (revent_ & EPOLLOUT)
            {
                if (writeCb_) writeCb_();
            }
            isHandlingEvent_ = false;
        }
    }
}