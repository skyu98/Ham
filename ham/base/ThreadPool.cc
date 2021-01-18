#include <functional>
#include <assert.h>
#include <stdio.h>
#include <memory>
#include "../base/ThreadPool.h"
#include "../base/Exception.h"

using namespace ham;

ThreadPool::ThreadPool(const string& name)
  : mutex_(),
    notEmpty_(),
    name_(name),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
  if (running_)
  {
    stop();
  }
}

void ThreadPool::start(int numThreads)
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
  {
    threads_.emplace_back(std::thread(std::mem_fn(&ThreadPool::runInThread))); 
  }
}

void ThreadPool::stop()
{
  {
  std::unique_lock<std::mutex> lock(mutex_);
  running_ = false;
  notEmpty_.notify_all();         // 唤醒所有阻塞的线程，通过running_=false，让所有线程执行完毕
  }
  for(auto& thr : threads_)
  {
    thr.join(); 
  }
             
}

void ThreadPool::run(Task task)
{
  if (threads_.empty())    // 如果线程池为空，说明就只有当前进程（生产者）可用，直接执行任务
  {
    task();
  }
  else      // 否则，向任务队列添加任务并唤醒阻塞的线程
  {
    {
      std::unique_lock<std::mutex> lock(mutex_);    
      queue_.emplace_back(std::move(task));
    }  
    notEmpty_.notify_one();
  }
}

ThreadPool::Task ThreadPool::take()
{
  std::unique_lock<std::mutex> lock(mutex_);
  // always use a while-loop, due to spurious wakeup
  while (queue_.empty() && running_)     
  {
    notEmpty_.wait(lock);
  }
  Task task;
  if(!queue_.empty())
  {
    task = queue_.front();    // 取出任务
    queue_.pop_front();
  }
  return task;
}

void ThreadPool::runInThread()
{
  try
  {
    while (running_)
    {
      /* 有任务做任务，没任务阻塞等待 */
      Task task(take());    
      if (task)
      {
        task();
      }
    }
  }
  catch (const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw; // rethrow
  }
}

