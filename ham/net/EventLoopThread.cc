#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
namespace ham
{
    namespace net
    {
        EventLoopThread::EventLoopThread(const ThreadInitCallback& cb) 
            : exiting_(false),
              initCallback_(cb),
              thread_(std::bind(&EventLoopThread::threadFunc, this))
        {
            
        }
        
        EventLoopThread::~EventLoopThread() 
        {
            exiting_ = true;
            loop_->quit();
            thread_.join();
        }
        
        
        void EventLoopThread::threadFunc() 
        {
            auto loop = std::make_shared<EventLoop>();
            if(initCallback_)
            {
                initCallback_(loop);
            }

            loop_ = loop;
            loop->loop();
        }
    }
}