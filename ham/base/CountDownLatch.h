#ifndef __COUNTDOWNLATCH_H__
#define __COUNTDOWNLATCH_H__
#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>

namespace ham
{

class CountDownLatch : public boost::noncopyable
{
public:
    explicit CountDownLatch(int count);
    
    void wait();
    void countdown();
    int getCount() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    int count_;

};

}

#endif // __COUNTDOWNLATCH_H__