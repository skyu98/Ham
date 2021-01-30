#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__
#include "base/Timestamp.h"
#include "net/SocketOps.h"

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <boost/noncopyable.hpp>

namespace ham
{
namespace net
{

namespace CurrentThread
{
    //thread_local static pid_t currentThreadId;
    pid_t gettid();
}

class Channel;
class Epoller;

class EventLoop : public boost::noncopyable
{
    typedef std::vector<Channel*> ChannelList;
    typedef std::function<void()> Functor;

public:
    EventLoop();
    ~EventLoop();

    /* 不能跨线程 */
    void loop();
    /* 可以跨线程 */
    void quit();

    void updateChannel(Channel*);
    void removeChannel(Channel*);

    bool isInLoopThread() const { return threadId_ == CurrentThread::gettid(); }
    void abortNotInLoopThread();
    void assertInLoopThread();

    void runInLoop(const Functor& func);

private:
    void wakeup();
    void handleWakeupFd(Timestamp receiveTime);
    void queueInLoop(const Functor& pendingfunc);
    void doPendingFunctors();

    bool looping_;
    bool quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;

    pid_t threadId_;
    int wakeup_fd_;
    Timestamp epollerReturnTime_;
    std::mutex mutex_;

    std::unique_ptr<Epoller> epoller_;
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
    std::shared_ptr<Channel> currentActiveChannel_;
    std::vector<Functor> pendingFunctors_;
};
}
}

#endif // __EVENTLOOP_H__