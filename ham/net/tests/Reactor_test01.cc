#include "net/EventLoop.h"
#include "base/CurrentThread.h"
#include "base/Log.h"
#include <thread>

using namespace ham;
using namespace ham::net;

// extern pid_t tid(); 匿名命名空间下相当于static，不能跨文件；除非加extern

void threadFunc()
{
    std::cout << "In thread pid = " << std::this_thread::get_id()
            << " tid = " << CurrentThread::tid() << std::endl;
    EventLoop loop;
    loop.loop();
}

int main()
{
    // TODO: can this be a marco or func?
    if(!Logger::Instance().init("Logger", "./log.txt", Logger::logLevel::trace))
    {
        return -1;
    }
    std::cout << "In thread pid = " << std::this_thread::get_id()
            << " tid = " << CurrentThread::tid() << std::endl;
    EventLoop loop;

    std::thread t(threadFunc);
    loop.loop();

    t.join();
    return 0;

}