#include "net/Epoller.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "base/Log.h"
#include <errno.h>

namespace ham
{
    namespace net
    {
        Epoller::Epoller(EventLoop* loop) 
            : loop_(loop),
              epoll_fd_(epoll_create1(EPOLL_CLOEXEC)),
              retEpoll_event_(kInitEventListSize)
        {
            
        }
        
        Epoller::~Epoller() 
        {
            close(epoll_fd_);
        }
        
        // int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
        Timestamp Epoller::wait(const int kEpollTimeMs, std::vector<Channel*>& ActiveChannels) 
        {
            int numOfEvents = epoll_wait(epoll_fd_, retEpoll_event_.data(), 
                            static_cast<int>(retEpoll_event_.size()), kEpollTimeMs);
            int savedErrno = errno;  // 避免被之后的覆盖

            Timestamp now(Timestamp::now());
            if(numOfEvents > 0)
            {
                TRACE("{} events has happened...", numOfEvents);
                fillActiveChannels(numOfEvents, ActiveChannels);
                if(numOfEvents == static_cast<int>(retEpoll_event_.size()))
                    retEpoll_event_.resize(retEpoll_event_.size() * 2);   
            }
            else if(numOfEvents == 0)
            {
                TRACE("Nothing happened...");
            }
            else if(savedErrno != EINTR)
            {
                errno = savedErrno;
                ERROR("Error: Epoll_wait error : {}", errno);
            }
            return now;
        }
        
        void Epoller::updateChannel(Channel* channel) 
        {
            loop_->assertInLoopThread();
            auto status = channel->getStatus();
            int fd = channel->getFd();
            
            if(status == Channel::status::kNew || status == Channel::status::kDismissed)
            // a new one , add with EPOLL_CTL_ADD
            {
                if(status == Channel::status::kNew)
                {
                    assert(channelMap_.find(fd) == channelMap_.end());
                    channelMap_[fd] = channel;
                }
                else
                {
                    assert(channelMap_.find(fd) != channelMap_.end());
                    assert(channelMap_[fd] == channel);
                }
                channel->setStatus(Channel::status::kAdded);
                update(EPOLL_CTL_ADD, channel);
            }
        }
        
        void Epoller::removeChannel(Channel*) 
        {
            
        }
        
        void Epoller::update(int op, Channel* channel) 
        {
            struct epoll_event tmp;
            int fd = channel->getFd();
            tmp.data.ptr = static_cast<void*>(channel);
            tmp.data.fd = fd, tmp.events = channel->getEvent();

            TRACE("epoll_ctl op = {}, fd = {}, event = [{}]", 
                operationToString(op), fd, channel->eventsToString());

            if(::epoll_ctl(epoll_fd_, op, fd, &tmp) < 0) 
            {
                if(op == EPOLL_CTL_DEL) {
                    ERROR("epoll_ctl op = {}, fd = {} ", 
                        operationToString(op), fd);
                } else {
                    CRITICAL("epoll_ctl op = {}, fd = {} ", 
                        operationToString(op), fd);
                }
            }
        }
        
        void Epoller::fillActiveChannels(int numOfEvents, 
                                        std::vector<Channel*>& ActiveChannels) const
        {
            assert(static_cast<size_t>(numOfEvents) <= retEpoll_event_.size());
            for(const auto event : retEpoll_event_)
            {
                Channel* channel = static_cast<Channel*>(event.data.ptr);
#ifndef NDEBUG  //TODO: DO WE NEED THIS ?
                int fd = channel->getFd();
                auto it = channelMap_.find(fd);
                assert(it != channelMap_.end());
                assert(it->second == channel);
#endif
                channel->setRevent(event.events);  //TODO: DO WE NEED THIS ?
                ActiveChannels.emplace_back(std::move(channel));
            }
        }

        std::string Epoller::operationToString(int op)
        {
            switch(op) 
            {
            case EPOLL_CTL_ADD:
                return "ADD";
            case EPOLL_CTL_DEL:
                return "DEL";
            case EPOLL_CTL_MOD:
                return "MOD";
            default:
                return "Unknown Operation";
            }
        }
    }
}