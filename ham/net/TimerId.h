#ifndef __TIMERID_H__
#define __TIMERID_H__
#include <stdint.h>
#include "base/copyable.h"
#include <memory>
namespace ham
{
namespace net
{
class Timer;

class TimerId : public ham::copyable // TODO : what if we use a map<serialNum, Timer*> to replace this class ?
{

public:
    TimerId()
        : timer_(nullptr),
          serialNum_(0)
    {}

    TimerId(Timer* timer, int64_t serialNum)
        : timer_(std::make_shared<Timer>(timer)),
          serialNum_(serialNum)
    {}
    
    TimerId(std::shared_ptr<Timer> timer, int64_t serialNum)
        : timer_(timer),
          serialNum_(serialNum)
    {}
 
    friend class TimerQueue;

private:
    /* data */
    std::shared_ptr<Timer> timer_;
    int64_t serialNum_;
};

}
}

#endif // __TIMERID_H__