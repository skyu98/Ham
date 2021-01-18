#ifndef HAM_BASE_THREADPOOL_H
#define HAM_BASE_THREADPOOL_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <deque>
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "../base/Types.h"


namespace ham
{

class ThreadPool : boost::noncopyable
{
 public:
  typedef std::function<void ()> Task;

  explicit ThreadPool(const string& name = string());
  ~ThreadPool();

  void start(int numThreads);          // 这是一个固定个数的线程池，没有考虑动态伸缩
  void stop();

  void run(Task f); 

 private:
  void runInThread();
  Task take();

  std::mutex mutex_;
  std::condition_variable notEmpty_;
  string name_;
  std::vector<std::thread> threads_;      // 存放Thread指针
  std::deque<Task> queue_;          // 存放任务
  bool running_;
};

}

#endif
