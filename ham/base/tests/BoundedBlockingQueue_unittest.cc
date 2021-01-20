#include "../BoundedBlockingQueue.h"
#include <vector>
#include <thread>
#include <string>
#include <iostream>
#include <unistd.h>

using namespace ham;

class Test
{
public:
    Test(int numThread, size_t maxsize)
        : queue_(maxsize)
    {
        for(int i = 0;i < numThread;++i)
        {
            threads_.emplace_back(std::thread(&Test::threadFunc, this));
        }
    }

    void joinAll()
    {
        
        for(size_t i = 0;i < threads_.size();++i)
        {
            queue_.put("quit");
        }
        
        for(auto& t: threads_)
        {
            t.join();
        }

    }

    void run(int numOfTasks)
    {
        for(int i = 0;i < numOfTasks;++i)
        {
            std::string task = "task" + std::to_string(i);
            std::cout << task << " is put in queue" << std::endl;
            queue_.put(std::move(task));
        }
    }

private: 
    void threadFunc()
    {
        bool running_ = true;
        while (running_)
        {

        std::string task = queue_.take();
        running_ = (task != "quit");
  
        std::cout << "I'm thread " << std::this_thread::get_id()
                << ", I'm working on " <<  task 
                << std::endl;
        usleep(10000);
        }
        std::cout << "I'm quitting... " << std::endl;
    }

    std::vector<std::thread> threads_;
    BoundedBlockingQueue<std::string> queue_;
};

int main()
{
    Test t(5, 50);
    t.run(100);
    t.joinAll();
    return 0;
}
