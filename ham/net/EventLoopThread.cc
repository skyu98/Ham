#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
namespace ham
{
    namespace net
    {
        EventLoopThread::EventLoopThread(const ThreadInitCallback& cb) 
            : loop_(nullptr),
              exiting_(false),
              initCallback_(cb)
        {
            
        }
        
        EventLoopThread::~EventLoopThread() 
        {
            exiting_ = true;
            loop_->quit();
            thread_.join();
        }
        
        EventLoop* EventLoopThread::startLoop() 
        {
            thread_ = std::thread(std::bind(&EventLoopThread::threadFunc, this));
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while(!loop_)
                {
                    cond_.wait(lock);
                }
            }
            return loop_;
        }
        
        
        void EventLoopThread::threadFunc() 
        {
            EventLoop loop;
            if(initCallback_)
            {
                initCallback_(&loop);
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                // loop_ 指针指向一个栈上的对象，threadFunc函数退出后，这个指针就失效了
                loop_ = &loop;
                cond_.notify_one();
            }

            loop.loop();

            loop_ = nullptr; // 这里一定要释放，否则会在主线程释放，则会报错
        }
    }
}