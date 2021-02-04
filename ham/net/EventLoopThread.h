#ifndef __EVENTLOOPTHREAD_H__
#define __EVENTLOOPTHREAD_H__
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <boost/noncopyable.hpp>
namespace ham
{
namespace net
{
class EventLoop;

class EventLoopThread : public boost::noncopyable
{
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
public:
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    /* data */
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    ThreadInitCallback initCallback_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

}
}

#endif // __EVENTLOOPTHREAD_H__