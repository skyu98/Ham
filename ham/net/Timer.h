#ifndef __TIMER_H__
#define __TIMER_H__
#include "base/Timestamp.h"
#include "base/Atomic.h"
#include "net/Callbacks.h"
#include <atomic>
#include <boost/noncopyable.hpp>
namespace ham
{
namespace net
{

class Timer : public boost::noncopyable
{
public:
    Timer(const TimerCallback& cb, Timestamp when, double interval);
    ~Timer() = default;

    void run() const { callback_(); }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return serialNum_; }

    void restart(Timestamp now);

    static int64_t numCreated() { return static_cast<int64_t>(s_numCreated_); }



private:
    /* data */
    const TimerCallback callback_;
    Timestamp expiration_; 
    const double interval_;
    const bool repeat_;
    const int64_t serialNum_; 

    static AtomicInt64 s_numCreated_;
};

}

}

#endif // __TIMER_H__