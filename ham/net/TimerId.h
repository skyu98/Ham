#ifndef __TIMERID_H__
#define __TIMERID_H__
#include <stdint.h>
#include "base/copyable.h"
#include "net/Timer.h"
#include <memory>
namespace ham
{
namespace net
{

class TimerId : public ham::copyable // TODO : what if we use a map<serialNum, Timer*> to replace this class ?
{

public:
    TimerId()
        : timer_(nullptr),
          serialNum_(0)
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