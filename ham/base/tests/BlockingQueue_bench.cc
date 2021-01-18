#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../CurrentThread.h"
#include "../Timestamp.h"

#include <thread>
#include <functional>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>

class Bench
{
 public:
  Bench(int numThreads)
    : latch_(numThreads)
  {
    for (int i = 0; i < numThreads; ++i)
    {
      threads_.emplace_back(std::thread(std::mem_fn(&Bench::threadFunc)));
    }
  }

  void run(int times)
  {
    printf("waiting for count down latch\n");
    latch_.wait();
    printf("all threads started\n");
    for (int i = 0; i < times; ++i)
    {
      ham::Timestamp now(ham::Timestamp::now());
      queue_.put(now);
      usleep(1000);
    }
  }

  void joinAll()
  {
    for (size_t i = 0; i < threads_.size(); ++i)
    {
      queue_.put(ham::Timestamp::invalid());
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

    std::map<int, int> delays;
    latch_.countDown();
    bool running = true;
    while (running)
    {
      ham::Timestamp t(queue_.take());
      ham::Timestamp now(ham::Timestamp::now());
      if (t.valid())
      {
        int delay = static_cast<int>(timeDifference(now, t) * 1000000);
        // printf("tid=%d, latency = %d us\n",
        //        ham::CurrentThread::tid(), delay);
        ++delays[delay];
      }
      running = t.valid();
    }

    printf("tid=%d, %s stopped\n",
           ham::CurrentThread::tid(),
           ham::CurrentThread::name());
    for (std::map<int, int>::iterator it = delays.begin();
        it != delays.end(); ++it)
    {
      printf("tid = %d, delay = %d, count = %d\n",
             ham::CurrentThread::tid(),
             it->first, it->second);
    }
  }

  ham::BlockingQueue<ham::Timestamp> queue_;
  ham::CountDownLatch latch_;
  std::vector<std::thread> threads_;
};

int main(int argc, char* argv[])
{
  int threads = argc > 1 ? atoi(argv[1]) : 1;

  Bench t(threads);
  t.run(10000);
  t.joinAll();
}
