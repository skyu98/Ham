#ifndef __EVENTLOOPTHREADPOOL_H__
#define __EVENTLOOPTHREADPOOL_H__
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include <boost/noncopyable.hpp>
namespace ham
{
namespace net
{
class EventLoopThread;
class EventLoop;

class EventLoopThreadPool : public boost::noncopyable
{
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
    typedef std::unique_ptr<EventLoopThread> EventLoopThreadPtr;
public:
    
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();

    void setNumOfThreads(int numOfThreads) { numOfThreads_ = numOfThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;
    bool started_;
    int numOfThreads_;
    int nextLoopIdx_;
    std::vector< EventLoopThreadPtr> threads_;
    std::vector<EventLoop*> loops_;
    
};

}
}
#endif // __EVENTLOOPTHREADPOOL_H__