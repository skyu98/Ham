#include "net/Epoller.h"

namespace ham
{
    namespace net
    {
        Epoller::Epoller(EventLoop* loop, int size) 
            : loop_(loop),
              epoll_fd_(epoll_create1(size))
        {
            
        }
        
        Epoller::~Epoller() 
        {
            
        }
        
        // int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
        void Epoller::wait(const int kEpollTimeMs, std::vector<Channel*>& ActiveChannels) 
        {
            epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
        }
        
        void Epoller::update(int op, Channel* channel) 
        {
            struct epoll_event tmp;
            tmp.data.ptr = channel, tmp.data.fd = channel->fd(), tmp.events = EPOLLIN;

            epoll_ctl(epoll_fd_, op, channel->fd(), &tmp);
        }
        
        void Epoller::fillActiveChannels(std::vector<Channel*>& ActiveChannels) 
        {
            
        }
    }
}