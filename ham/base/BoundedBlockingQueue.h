#ifndef __BOUNDEDBLOCKINGQUEUE_H__
#define __BOUNDEDBLOCKINGQUEUE_H__
#include <deque>
#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>

namespace ham
{

template<typename T>
class BoundedBlockingQueue : public boost::noncopyable
{
public:
    BoundedBlockingQueue(size_t maxsize);

    void put(const T& task);
    void put(T&& task);

    T take();
    size_t size() const;
    size_t capacity() const;
    bool full() const;
    bool empty() const;
    

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    boost::circular_buffer<T>  queue_;         // 环形队列，重复利用内存。（这里内部用指针实现的）
};

template<typename T>
BoundedBlockingQueue<T>::BoundedBlockingQueue(size_t maxsize) 
    : queue_(maxsize)
{}

template<typename T>
void BoundedBlockingQueue<T>::put(const T& task) 
{
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.full())
    {
        notFull_.wait(lock);
    }
    queue_.push_back(task);
    notEmpty_.notify_one();
}

template<typename T>
void BoundedBlockingQueue<T>::put(T&& task) 
{
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.full())
    {
        notFull_.wait(lock);
    }
    queue_.push_back(std::move(task)); // task本身是个左值，是个右值引用；move后变为右值
    notEmpty_.notify_one();
}

template<typename T>
T BoundedBlockingQueue<T>::take() 
{
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.empty())
    {
        notEmpty_.wait(lock);
    }
    T task(std::move(queue_.front()));
    queue_.pop_front();
    notFull_.notify_one();
    return task;
}

template<typename T>
inline size_t BoundedBlockingQueue<T>::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

template<typename T>
inline size_t BoundedBlockingQueue<T>::capacity() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.capacity();
}

template<typename T>
inline bool BoundedBlockingQueue<T>::full() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.full();
}

template<typename T>
inline bool BoundedBlockingQueue<T>::empty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}


}


#endif // __BOUNDEDBLOCKINGQUEUE_H__