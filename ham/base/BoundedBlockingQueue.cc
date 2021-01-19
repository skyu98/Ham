#include "BoundedBlockingQueue.h"


namespace ham
{
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