#ifndef __TIMERID_H__
#define __TIMERID_H__
#include <stdint.h>
namespace ham
{
namespace net
{
class Timer;

class TimerId
{

public:
    TimerId()
        : timer_(nullptr),
          serialNum_(0)
    {}

    TimerId(Timer* timer, int64_t serialNum)
        : timer_(timer),
          serialNum_(serialNum)
    {}
    
    friend class TimerQueue;

private:
    /* data */
    Timer* timer_;
    int64_t serialNum_;
};

}
}

#endif // __TIMERID_H__