#include "net/EventLoop.h"
#include "net/CurrentThread.h"
#include "base/Log.h"
#include <thread>

using namespace ham;
using namespace ham::net;

int cnt = 0;
EventLoop* g_loop;

void printTid() {
    std::cout << "pid = " << std::this_thread::get_id() << " tid = " << CurrentThread::tid() << std::endl;
    std::cout << "now : " << Timestamp::now().toString() << std::endl;
}

void print(const std::string& msg) {
    std::cout << "msg " << msg << " : " << Timestamp::now().toString() << std::endl;
    if(++cnt == 20) {
        g_loop->quit();
    }
}

void cancel(TimerId timer, const std::string& name) {
    g_loop->cancelTimer(timer);
    std::cout << name <<" cancelled at " << Timestamp::now().toString() << std::endl;
}

int main() {
    if (!Logger::Instance().init("log", "logs/test.txt", Logger::logLevel::trace)) {
		return 1;
	}
    printTid();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        EventLoop loop;
        g_loop = &loop;
        std::cout << "main" << std::endl;
        loop.runAfter(1, std::bind(print, "once 1"));
        loop.runAfter(1.5, std::bind(print, "once 1.5"));
        loop.runAfter(2.5, std::bind(print, "once 2.5"));
        loop.runAfter(3.5, std::bind(print, "once 3.5"));
        TimerId t45 = loop.runAfter(4.5, std::bind(print, "once 4.5"));
        // 4.2时把4.5注销掉
        loop.runAfter(4.2, std::bind(cancel, t45, "t45"));
        // 已经取消，这里已没什么效果       
        loop.runAfter(4.8, std::bind(cancel, t45, "t45"));
        loop.runEvery(2, std::bind(print, "every2"));
        TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
        loop.runAfter(9.001, std::bind(cancel, t3, "t3"));

        loop.loop();
        std::cout << "main loop exits" << std::endl;
    }

}
