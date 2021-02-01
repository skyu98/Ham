#include "net/EventLoop.h"
#include "net/TimerQueue.h"
#include "net/Timer.h"
#include "net/TimerId.h"
#include "net/Channel.h"
#include "base/Util.h"
namespace ham
{
    namespace net
    {
        namespace detail
        {
            int create_alarmFd();
            void resetAlarmFd(int alarmFd, Timestamp when);
        }

        TimerQueue::TimerQueue(EventLoop* loop)
            : loop_(loop),
              alarmFd_(detail::create_alarmFd()),
              alarmChannel_(util::make_unique<Channel>(loop, alarmFd_)),
              earliestChanged_(false),
              callingExpiredTimers_(false)
        {
            alarmChannel_->setReadCallback(std::bind(&TimerQueue::handleRead, this, std::placeholders::_1));
            alarmChannel_->enableReading();
        }
        
        TimerQueue::~TimerQueue() 
        {
            alarmChannel_->disableAll();
            alarmChannel_->remove();
            ::close(alarmFd_);
        }
        
        TimerId TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval) 
        {
            auto timer = std::make_shared<Timer>(cb, when, interval);
            loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
            return TimerId(timer, timer->serialNum());
        }
        
        void TimerQueue::cancelTimer(TimerId id) 
        {
            loop_->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, id));
        }
        
        void TimerQueue::handleRead(Timestamp alarmTime) 
        {
            
        }
        
        void TimerQueue::addTimerInLoop(std::shared_ptr<Timer> timer) 
        {
            loop_->assertInLoopThread();
            insert(timer);
            if(earliestChanged_)
            {
                detail::resetAlarmFd(alarmFd_, timer->expiration());
            }
        }
        
        void TimerQueue::cancelTimerInLoop(TimerId id) 
        {
            loop_->assertInLoopThread();
            auto timer = id.timer_;
            auto it = timerList_.find(timeEntry(timer->expiration(), timer));
            if(it != timerList_.end())
            {
                timerList_.erase(it);
            }
            else if(callingExpiredTimers_)
            {
                cancelingTimers_.insert(timer);
            }
        }
        
        void TimerQueue::insert(std::shared_ptr<Timer> timer) 
        {
            loop_->assertInLoopThread();
            earliestChanged_ = false;
            Timestamp when = timer->expiration();
            auto it = timerList_.begin();

            if(it == timerList_.end() || when < it->first)
            {
                earliestChanged_ = true;
            }
            {
                auto res = timerList_.insert(timeEntry(when, timer));
                assert(res.second == true);
            }
        }
    }
}