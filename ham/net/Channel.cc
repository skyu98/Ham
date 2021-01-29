#include "net/EventLoop.h"
#include "net/Channel.h"

namespace ham
{
    namespace net
    {
        Channel::Channel(EventLoop* loop, int fd)
            : loop_(loop),
              fd_(fd),
              event_(0),
              revent_(0),
              isHandlingEvent_(false)
        {
            
        }
        
        void Channel::update() 
        {
            loop_->updateChannel(this);
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