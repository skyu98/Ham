#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__
#include "base/Timestamp.h"
#include "net/SocketOps.h"
#include "net/Callbacks.h"
#include "net/TimerId.h"
#include "base/Log.h"
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <boost/noncopyable.hpp>

namespace ham
{
namespace net
{

class Channel;
class Epoller;
class TimerQueue;

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

    bool isInLoopThread() const;
    void abortNotInLoopThread();
    void assertInLoopThread();

    void runInLoop(const Functor& func);
    void queueInLoop(const Functor& pendingfunc);

    TimerId runAt(const Timestamp& time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);
    void cancelTimer(TimerId);

private:
    void wakeup();
    void handleWakeupFd(Timestamp receiveTime);
    void doPendingFunctors();

    bool looping_;
    bool quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;

    pid_t threadId_;
    int wakeup_fd_;
    Timestamp epollerReturnTime_;
    mutable std::mutex mutex_;

    std::unique_ptr<Epoller> epoller_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::unique_ptr<TimerQueue> timerQueue_;
    ChannelList activeChannels_;
    // std::shared_ptr<Channel> currentActiveChannel_;
    std::vector<Functor> pendingFunctors_;
};
}
}

#endif // __EVENTLOOP_H__