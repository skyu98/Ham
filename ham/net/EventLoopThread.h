#ifndef __EVENTLOOPTHREAD_H__
#define __EVENTLOOPTHREAD_H__
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <boost/noncopyable.hpp>
namespace ham
{
namespace net
{
class EventLoop;

class EventLoopThread : public boost::noncopyable
{
    typedef std::function<void(std::shared_ptr<EventLoop>)> ThreadInitCallback;
public:
    EventLoopThread(const ThreadInitCallback& cb);
    ~EventLoopThread();

    std::shared_ptr<EventLoop> getLoop() const { return loop_; }

private:
    /* data */
    void threadFunc();

    bool exiting_;
    ThreadInitCallback initCallback_;
    std::shared_ptr<EventLoop> loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

}
}

#endif // __EVENTLOOPTHREAD_H__