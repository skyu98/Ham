#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__
#include <memory>
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>

namespace ham
{
namespace net
{
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

    bool EventLoop::isInLoopThread() const;
    void abortNotInLoopThread();
    void assertInLoopThread();

private:
    void handleWakeupFd();

    bool looping_;
    bool quit_;
    bool eventHandling_;

    pid_t threadId_;
    int wakeup_fd_;

    std::unique_ptr<Epoller> epoller_;
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
    std::shared_ptr<Channel> currentActiveChannel_;
};
}
}

#endif // __EVENTLOOP_H__