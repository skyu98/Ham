#ifndef HAM_BASE_COUNTDOWNLATCH_H
#define HAM_BASE_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>

namespace ham
{

class CountDownLatch : boost::noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  int count_;
};

}
#endif  // HAM_BASE_COUNTDOWNLATCH_H
