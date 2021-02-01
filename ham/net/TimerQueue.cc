#include "net/EventLoop.h"
#include "net/TimerQueue.h"
#include "net/Timer.h"
#include "net/TimerId.h"
#include "net/Channel.h"
#include "base/Util.h"
#include "base/Log.h"

#include <sys/timerfd.h>
namespace ham
{
    namespace net
    {
        namespace detail
        {
            int create_alarmFd()
            {
                int alarmFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
                if(alarmFd < 0)
                {
                    CRITICAL("Failed in create_alarmFd");
                }
                return alarmFd;
            }

            // 计算超时时刻与当前时间的时间差
            struct timespec howMuchTimeFromNow(Timestamp when)
            {
                int64_t microseconds = when.microsecondsFromEpoch()
                                        - Timestamp::now().microsecondsFromEpoch();
                if (microseconds < 100)
                {
                    microseconds = 100;
                }
                struct timespec ts;
                ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicrosecondsPerSecond);
                ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicrosecondsPerSecond) * 1000);
                return ts;
            }

            // 处理超时事件，超时后alarmFd变为可读，howmany表示超时的次数
            // 将事件读出来，避免陷入Loop忙碌状态
            void readAlarmfd(int alarmFd, Timestamp now) {
                uint64_t howmany;
                ssize_t n = ::read(alarmFd, &howmany, sizeof(howmany));
                TRACE("TimerQueue::handleRead {} at {}", howmany, now.toString());
                if(n != sizeof(howmany)) {
                    ERROR("TimerQueue::handleRead reads {} bytes instead of 8", n);
                }   
            }

            void resetAlarmFd(int alarmFd, Timestamp expiration)
            {
                // wake up loop by timerfd_settime()
                struct itimerspec newValue;
                struct itimerspec oldValue;
                bzero(&newValue, sizeof newValue);
                bzero(&oldValue, sizeof oldValue);
                newValue.it_value = howMuchTimeFromNow(expiration);
                int ret = ::timerfd_settime(alarmFd, 0, &newValue, &oldValue);
                if (ret)
                {
                    ERROR("timerfd_settime()");
                }
            }
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
            loop_->assertInLoopThread();
            Timestamp now(Timestamp::now());
            detail::readAlarmfd(alarmFd_, now);

            std::vector<timeEntry> expired = getExpired(now);

            cancelingTimers_.clear();
            callingExpiredTimers_ = true;
            for(const auto& entry : expired)
            {
                entry.second->run();
            }
            callingExpiredTimers_ = false;

            reset(expired, now);
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
                assert(res.second);
            }
        }
        
        // Return value optimization
        std::vector<TimerQueue::timeEntry> TimerQueue::getExpired(Timestamp now) 
        {
            timeEntry helper(now, std::shared_ptr<Timer>());
            auto lastExpired = timerList_.lower_bound(helper);
            if(lastExpired != timerList_.end() && lastExpired->first == now)
                ++lastExpired;
            assert(lastExpired == timerList_.end() || lastExpired->first > now);

            std::vector<timeEntry> expired;
            std::copy(timerList_.begin(), lastExpired, std::back_inserter(expired));

            timerList_.erase(timerList_.begin(), lastExpired);
            return expired;
        }
        
        void TimerQueue::reset(const std::vector<timeEntry>& expired, Timestamp now) 
        {
            for(const auto& expiredTimer : expired)
            {
                if(expiredTimer.second->repeat() && 
                    cancelingTimers_.find(expiredTimer.second) == cancelingTimers_.end())
                {
                    expiredTimer.second->restart(now);
                    insert(expiredTimer.second);
                }
            }

            Timestamp nextExpired;
            if(!timerList_.empty())
                nextExpired = timerList_.begin()->second->expiration();
            
            if(nextExpired.valid())
                detail::resetAlarmFd(alarmFd_, nextExpired);
        }
    }
}