#include "../CountDownLatch.h"
#include <iostream>
#include <thread>
#include <vector>
#include <assert.h>
#include <unistd.h>

using namespace ham;

class Test
{
public:
    Test(int numThread)
        : latch_(1)
    {
        for(int i = 0;i < numThread;++i)
        {
            threads_.emplace_back(std::thread(&Test::threadFunc, this));
        }
    }

    void joinAll()
    {
        for(auto& t: threads_)
        {
            t.join();
        }
    }

    void countDown()
    {
        latch_.countdown();
    }

private: 
    void threadFunc()
    {
        /* this_thread::inline thread::id
            get_id() noexcept { return thread::id(__gthread_self()); }
        */
        std::cout << "I'm thread " << std::this_thread::get_id()
                << ", I'm waiting for count down latch..."
                << std::endl; 

        latch_.wait();
  
        std::cout << "I'm thread " << std::this_thread::get_id()
                << ", I'm working..."
                << std::this_thread::get_id() 
                << std::endl;
    }

    std::vector<std::thread> threads_;
    CountDownLatch latch_;
};

int main()
{
    std::cout << "I'm the main thread " << std::this_thread::get_id() << std::endl;
    Test T(5);
    sleep(5);
    T.countDown();
    T.joinAll();
    return 0;
}
