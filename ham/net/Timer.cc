#include "net/Timer.h"
namespace ham
{
    namespace net
    {
        Timer::Timer(const TimerCallback& cb, Timestamp when, double interval) 
            : callback_(cb),
              expiration_(when),
              interval_(interval),
              repeat_(interval > 0.0),
              serialNum_(++s_numCreated_)
        {
            
        }
        
        void Timer::restart(Timestamp now) 
        {
            if(repeat_)
            {
                expiration_ = addTime(now, interval_);
            }
            else
            {
                expiration_ = Timerstamp::invalid();
            }
        }
    }
}