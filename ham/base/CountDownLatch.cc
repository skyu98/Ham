#include "CountDownLatch.h"

namespace ham
{
    CountDownLatch::CountDownLatch(int count) 
        : count_(count)
    {}
    
    void CountDownLatch::wait()
    {
        std::unique_lock<std::mutex> lock(mutex_); // 搭配wait，效率不如lockguard高
        while(count_ > 0)
            cond_.wait(lock);
    }
    
    void CountDownLatch::countdown() 
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            --count_;
        }
        if(0 == count_)
            cond_.notify_all();
    }
    
    int CountDownLatch::getCount() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
}