#include "../base/CountDownLatch.h"

using namespace ham;

CountDownLatch::CountDownLatch(int count)
  : count_(count)
{
}

void CountDownLatch::wait()
{
  std::unique_lock<std::mutex> lock(mutex_);     // 离开本函数时，lock析构进行解锁
  while (count_ > 0) {        
    condition_.wait(lock);    // 等待倒计时
  }
}

void CountDownLatch::countDown()
{
  std::unique_lock<std::mutex> lock(mutex_);
  --count_;         
  if (count_ == 0) {
    condition_.notify_all();   // 倒计时完毕，通知所有等待的线程
  }
}

int CountDownLatch::getCount() const
{
  std::unique_lock<std::mutex> lock(mutex_);     // 成员变量mutex_状态可以在const函数中被修改，因为它是mutable的
  return count_;                   // count就不能被改变     
}

