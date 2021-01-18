#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../CurrentThread.h"

#include <thread>
#include <functional>
#include <vector>
#include <string>
#include <stdio.h>

class Test
{
 public:
  Test(int numThreads)
    : latch_(numThreads)
  {
    printf("num is %d\n", numThreads);
    for (int i = 0; i < numThreads; ++i)
    {
      // char name[32];
      // snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(std::thread(&Test::threadFunc, this));
    }
  }

  void run(int times)
  {
    printf("waiting for count down latch\n");
    latch_.wait();
    printf("all threads started\n");
    for (int i = 0; i < times; ++i)
    {
      char buf[32];
      snprintf(buf, sizeof buf, "hello %d", i);
      queue_.put(buf);
      printf("tid=%d, put data = %s, size = %zd\n", ham::CurrentThread::tid(), buf, queue_.size());
    }
  }

  void joinAll()
  {
    for (size_t i = 0; i < threads_.size(); ++i)
    {
      queue_.put("stop");
    }

    for(auto it = threads_.begin();it != threads_.end();++it)
    {
      it->join();
    }
  }

 private:

  void threadFunc()
  {
    printf("tid=%d, %s started\n",
           ham::CurrentThread::tid(),
           ham::CurrentThread::name());

    latch_.countDown();
    bool running = true;
    while (running)
    {
      std::string d(queue_.take());
      printf("tid=%d, get data = %s, size = %zd\n", ham::CurrentThread::tid(), d.c_str(), queue_.size());
      running = (d != "stop");
    }

    printf("tid=%d, %s stopped\n",
           ham::CurrentThread::tid(),
           ham::CurrentThread::name());
  }

  ham::BlockingQueue<std::string> queue_;
  ham::CountDownLatch latch_;
  std::vector<std::thread> threads_;
};

int main()
{
  printf("pid=%d, tid=%d\n", ::getpid(), ham::CurrentThread::tid());
  Test t(5);
  t.run(100);
  t.joinAll();
}
