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
            else
            // an existing one , add with EPOLL_CTL_MOD/DEL
            {
                assert(channelMap_.find(fd) != channelMap_.end());
                assert(channelMap_[fd] == channel);
                assert(status == Channel::status::kAdded);
                if(channel->isNoneEvent())
                {
                    update(EPOLL_CTL_DEL, channel);
                    channel->setStatus(Channel::status::kDismissed);
                }
                else
                    update(EPOLL_CTL_MOD, channel);
            }
            
        }
        
        void Epoller::removeChannel(Channel* channel) 
        {
            loop_->assertInLoopThread();

            int fd = channel->getFd();
            auto status = channel->getStatus();

            TRACE("fd = {}, is going to be removed...", fd);
            assert(channel->isNoneEvent());
            assert(channelMap_.find(fd) != channelMap_.end());
            assert(channelMap_[fd] == channel);
            assert(status == Channel::status::kAdded ||
                    status == Channel::status::kDismissed);
            
            size_t n = channelMap_.erase(fd);
            assert(n == 1);

            if(status == Channel::status::kAdded)
                update(EPOLL_CTL_DEL, channel);
            channel->setStatus(Channel::status::kNew);
        }
        
        void Epoller::update(int op, Channel* channel) 
        {
            struct epoll_event tmp;
            bzero(&tmp, sizeof(tmp));
            int fd = channel->getFd();
            
            tmp.data.ptr = static_cast<void*>(channel);
            tmp.events = channel->getEvent();

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
            // TRACE("ptr is {}", fmt::ptr(tmp.data.ptr));
            // TRACE("channel is {}", fmt::ptr(channel));
        }
        
        void Epoller::fillActiveChannels(int numOfEvents, 
                                        std::vector<Channel*>& ActiveChannels) const
        {
            assert(static_cast<size_t>(numOfEvents) <= retEpoll_event_.size());
            for(int i = 0;i < numOfEvents;++i)   // do not use for(const auto& event : retEpoll_event_)
            {
                Channel* channel = static_cast<Channel*>(retEpoll_event_[i].data.ptr);
#ifndef NDEBUG  //TODO: DO WE NEED THIS ?
                int fd = channel->getFd();
                auto it = channelMap_.find(fd);
                assert(it != channelMap_.end());
                assert(it->second == channel);
#endif
                channel->setRevent(retEpoll_event_[i].events);  //TODO: DO WE NEED THIS ?
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