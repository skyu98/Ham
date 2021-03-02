#include "EventLoop.h"
#include "net/Channel.h"
#include "net/Epoller.h"
#include "net/TimerQueue.h"
#include "base/Log.h"
#include "base/Util.h"
#include "net/CurrentThread.h"
#include <sys/eventfd.h>
#include <thread>

using namespace ham;
using namespace ham::net;

thread_local pid_t CurrentThread::currentThreadId = 0;

namespace
{
thread_local EventLoop* t_loopInThisThread = nullptr;
const int kEpollTimeMs = 10000;

int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    ERROR("Failed in eventfd");
  }
  return evtfd;
}
}

class Log
{
    Log()
    {
        
    }
}

EventLoop::EventLoop() 
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(static_cast<pid_t>(CurrentThread::tid())),
      wakeup_fd_(::createEventfd()),
      epoller_(util::make_unique<Epoller>(this)),
      wakeupChannel_(util::make_unique<Channel>(this, wakeup_fd_)),
      timerQueue_(util::make_unique<TimerQueue>(this))
{
    TRACE("EventLoop created {} ", fmt::ptr(this));
    if(t_loopInThisThread)
    {
        CRITICAL("Another EventLoop {} exists in this Thread( tid = {} ) ...", fmt::ptr(t_loopInThisThread), CurrentThread::tid()); 
    }
    else
    {
        t_loopInThisThread = this;
    }
    
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeupFd, this, 
                                    std::placeholders::_1));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() 
{
    assert(quit_);
    DEBUG("EventLoop {} of thread {} destructs", 
        fmt::ptr(this), threadId_)
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeup_fd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() 
{
    looping_ = true;
    while(!quit_)
    {
        activeChannels_.clear();
        epollerReturnTime_ = epoller_->wait(kEpollTimeMs, activeChannels_); 
        eventHandling_ = true;
        for(const auto& channel : activeChannels_)
        {
            channel->handleEvent(epollerReturnTime_);
        }
        eventHandling_ = false;
        doPendingFunctors();
    }
    TRACE("EventLoop {} stop looping", fmt::ptr(this));
    looping_ = false;
}

void EventLoop::quit() 
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup(); // 跨线程调用时，可能正在handleEvent，也可能wait阻塞住，所以要去唤醒，所以我们需要个唤醒通道
    }
}

void EventLoop::updateChannel(Channel* channel) 
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) 
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoller_->removeChannel(channel);
}

bool EventLoop::isInLoopThread() const
{
    return threadId_ == CurrentThread::tid();
}

void EventLoop::abortNotInLoopThread() 
{
    CRITICAL("EventLoop::abortNotInLoopThread - EventLoop {} was created in threadId_ = {}, current thread id = {}!!!",
            fmt::ptr(this), threadId_, CurrentThread::tid());
}

void EventLoop::assertInLoopThread() 
{
    if(!isInLoopThread())
    {
        abortNotInLoopThread();
    }
}


void EventLoop::runInLoop(const Functor& func) 
{
    if(isInLoopThread())
    {
        func();
    }
    else
    {
        queueInLoop(func);
    }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) 
{
    return timerQueue_->addTimer(cb, time, 0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) 
{
    Timestamp when = addTime(Timestamp::now(), delay);
    return runAt(when, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) 
{
    Timestamp when = addTime(Timestamp::now(), interval);
    return timerQueue_->addTimer(cb, when, interval);
}

void EventLoop::cancelTimer(TimerId id) 
{
    timerQueue_->cancelTimer(id);
}

void EventLoop::queueInLoop(const Functor& pendingFunc) 
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(pendingFunc);
    }
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
        
  
}

void EventLoop::wakeup() 
{
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeup_fd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        ERROR("EventLoop::wakeup() writes {} bytes instead of 8", n);
    }
}

void EventLoop::handleWakeupFd(Timestamp receiveTime) 
{
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeup_fd_, &one, sizeof(one));
    if(n != sizeof(one)) {
        ERROR("EventLoop::handleRead reads {} bytes instead of 8 ", n);
    }   
}

void EventLoop::doPendingFunctors() 
{
    std::vector<Functor> Functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Functors.swap(pendingFunctors_);
    }
    for(const auto& func : Functors)
    {
        func();
    }
    callingPendingFunctors_ = false;
}
