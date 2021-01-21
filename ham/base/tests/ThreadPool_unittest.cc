#include "../ThreadPool.h"
#include "../CountDownLatch.h"
#include <iostream>

using namespace ham;

void print()
{
    std::cout << "tid: " << std::this_thread::get_id() << std::endl;
}

void printTaskId(const std::string& id)
{
    std::cout << "tid: " << std::this_thread::get_id()
            << " doing task id: " << id << std::endl;
}

int main()
{
    ThreadPool pool("Main pool");
    pool.setInitCallback(print);
    pool.setMaxQueueSize(50);
    pool.start(5);

    for(int i = 0;i < 100;++i)
    {
        std::string name = std::to_string(i);
        pool.run(std::bind(&printTaskId, name));
    }

    CountDownLatch latch(1);
    pool.run(std::bind(&CountDownLatch::countdown, &latch));
    printf("wait!!!\n");
    latch.wait();

    printf("wait over!!!\n");
    pool.stop();
    return 0;
}