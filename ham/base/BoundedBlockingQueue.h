#ifndef HAM_BASE_BOUNDEDBLOCKINGQUEUE_H
#define HAM_BASE_BOUNDEDBLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <assert.h>

namespace ham
{

template<typename T>
class BoundedBlockingQueue : boost::noncopyable
{
 public:
  explicit BoundedBlockingQueue(int maxSize)
    : mutex_(),
      notEmpty_(),
      notFull_(),
      queue_(maxSize) 
  {
  }

  void put(const T& x)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.full())            // 相比无边界队列，多了对队列是否为满的判断。满了则等待消费。
    {
      notFull_.wait(lock);
    }
    assert(!queue_.full());
    queue_.push_back(x);
    notEmpty_.notify_one(); // TODO: move outside of lock
  }

  T take()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty())
    {
      notEmpty_.wait(lock);
    }
    assert(!queue_.empty());
    T front(queue_.front());
    queue_.pop_front();
    notFull_.notify_one(); // TODO: move outside of lock
    return front;
  }

  bool empty() const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  bool full() const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.full();
  }

  size_t size() const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.capacity();
  }

 private:
  mutable std::mutex         mutex_;
  std::condition_variable    notEmpty_;
  std::condition_variable    notFull_;
  boost::circular_buffer<T>  queue_;         // 环形队列，重复利用内存。（这里不是用取模实现的）
};

}

#endif  // HAM_BASE_BOUNDEDBLOCKINGQUEUE_H
