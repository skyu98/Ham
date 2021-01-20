#ifndef __BLOCKINGQUEUE_H__
#define __BLOCKINGQUEUE_H__
#include <deque>
#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>

namespace ham
{

template<typename T>
class BlockingQueue : public boost::noncopyable
{
public:
    BlockingQueue() = default;

    void put(const T& task);
    void put(T&&);
    T take();
    size_t size() const;
    

private:
    std::deque<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
};

template<typename T>
void BlockingQueue<T>::put(const T& task) 
{   
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace_back(task);
    }
    notEmpty_.notify_one();
}

template<typename T>
void BlockingQueue<T>::put(T&& task) 
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace_back(std::move(task));
    }
    notEmpty_.notify_one();
}

template<typename T>
T BlockingQueue<T>::take() 
{
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.empty())
    {
        notEmpty_.wait(lock);
    }
    T task(std::move(queue_.front()));
    queue_.pop_front();
    return task;
}

template<typename T>
size_t BlockingQueue<T>::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

}


#endif // __BLOCKINGQUEUE_H__