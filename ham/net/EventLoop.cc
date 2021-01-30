#include "EventLoop.h"
#include "net/Channel.h"
#include "net/Epoller.h"
#include "base/Log.h"
#include "base/Util.h"
#include <sys/eventfd.h>
#include <thread>

using namespace ham;
using namespace ham::net;

namespace CurrentThread
{
thread_local static pid_t currentThreadId = 0;

pid_t gettid()
{
  if(currentThreadId == 0)
    currentThreadId = ::util::gettid();
  return currentThreadId;
}
}


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
    abort();
  }
  return evtfd;
}
}

EventLoop::EventLoop() 
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::gettid()),
      wakeup_fd_(::createEventfd()),
      epoller_(util::make_unique<Epoller>(this)),
      wakeupChannel_(util::make_unique<Channel>(this, wakeup_fd_))
{
    TRACE("EventLoop created {} ", fmt::ptr(this));
    if(t_loopInThisThread)
    {
        CRITICAL("Another EventLoop {} exists in this Thread( tid = {} ) ...", fmt::ptr(t_loopInThisThread), CurrentThread::gettid()); 
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
    assert(quit_ == true);
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

void EventLoop::abortNotInLoopThread() 
{
    CRITICAL("EventLoop::abortNotInLoopThread - EventLoop {} was created in threadId_ = {}, current thread id = {}!!!",
            fmt::ptr(this), threadId_, CurrentThread::gettid());
    abort();
}

void EventLoop::assertInLoopThread() 
{
    if(!isInLoopThread())
        abortNotInLoopThread();
}


void EventLoop::runInLoop(const Functor& func) 
{
    if(isInLoopThread())
        func();
    queueInLoop(func);
}

void EventLoop::queueInLoop(const Functor& pendingFunc) 
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(pendingFunc);
    }
    if(!isInLoopThread() || callingPendingFunctors_)
        wakeup();
  
}

void EventLoop::wakeup() 
{
    uint64_t one = 1;
    ssize_t n = socket::write(wakeup_fd_, &one, sizeof(one));
    if(n != sizeof(one))
        ERROR("EventLoop::wakeup() writes {} bytes instead of 8", n);
}

void EventLoop::handleWakeupFd(Timestamp receiveTime) 
{
    uint64_t one = 1;
    ssize_t n = socket::read(wakeup_fd_, &one, sizeof(one));
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
