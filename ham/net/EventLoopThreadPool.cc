#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
#include "net/EventLoopThreadPool.h"
#include "base/Util.h"
#include "base/Types.h"
#include <assert.h>
namespace ham
{
    namespace net
    {
        EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
            : baseLoop_(baseLoop),
              started_(false),
              numOfThreads_(0),
              nextLoopIdx_(0)
        {
        }
        
        EventLoopThreadPool::~EventLoopThreadPool() 
        {
        }

        void EventLoopThreadPool::start(const ThreadInitCallback& cb)
        {
            assert(!started_);
            started_ = true;
            baseLoop_->assertInLoopThread();

            for(int i = 0;i < numOfThreads_;++i)
            {
                auto thread = util::make_unique<EventLoopThread>(cb);
                loops_.emplace_back(thread->startLoop());
                threads_.emplace_back(std::move(thread));
            }

            if(numOfThreads_ == 0 && cb)
            {
                cb(baseLoop_);
            }
        }

        EventLoop* EventLoopThreadPool::getNextLoop() 
        {
            baseLoop_->assertInLoopThread();
            EventLoop* nextLoop = baseLoop_;
            if(!loops_.empty())
            {
                nextLoop = loops_[nextLoopIdx_++];
                if(implicit_cast<size_t>(nextLoopIdx_) >= loops_.size())
                {
                    nextLoopIdx_ = 0;
                }
            }
            return nextLoop;
        }
       
    }
}