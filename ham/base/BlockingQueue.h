#ifndef HAM_BASE_BLOCKINGQUEUE_H
#define HAM_BASE_BLOCKINGQUEUE_H

#include <condition_variable>
#include <mutex>

#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>

namespace ham
{

template<typename T>   //模版类 
class BlockingQueue : boost::noncopyable       // 无边界
{
 public:
  BlockingQueue()
    : queue_()
  {
  }

  void put(const T& x)
  {
    {// TODO: move outside of lock。可以将lock和push_back放在同一小作用域内，缩小锁的粒度
      std::unique_lock<std::mutex> lock(mutex_);
      queue_.push_back(x);
    }
    notEmpty_.notify_one(); 
  }

  T take()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // always use a while-loop, due to spurious wakeup。防止一次唤醒多个线程取数据
    while (queue_.empty())
    {
      notEmpty_.wait(lock);
    }
    assert(!queue_.empty());
    T front(queue_.front());
    queue_.pop_front();
    return front;
  }

  size_t size() const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  mutable std::mutex mutex_;     // 同样，mutable修饰，便于在const函数内上锁
  std::condition_variable  notEmpty_;
  std::deque<T>     queue_;
};

}

#endif  // HAM_BASE_BLOCKINGQUEUE_H
