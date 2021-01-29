#include "EventLoop.h"
#include "net/Channel.h"
#include "net/Epoller.h"
#include "base/Log.h"
#include "base/Util.h"
#include <sys/eventfd.h>
#include <thread>

using namespace ham;
using namespace ham::net;

namespace
{
thread_local EventLoop* t_loopInThisThread = nullptr;
thread_local static pid_t currentThreadId = 0;

pid_t tid()
{
  if(currentThreadId == 0)
    currentThreadId = ::util::gettid();
  return currentThreadId;
}

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
      threadId_(::tid()),
      wakeup_fd_(::createEventfd()),
      epoller_(util::make_unique<Epoller>(this)),
      wakeupChannel_(util::make_unique<Channel>(wakeup_fd_))
{
    TRACE("EventLoop created {} ", fmt::ptr(this));
    if(t_loopInThisThread)
    {
        CRITICAL("Another EventLoop {} exists in this Thread( tid = {} ) ...", fmt::ptr(t_loopInThisThread), ::tid()); 
    }
    else
    {
        t_loopInThisThread = this;
    }
    
    wakeupChannel_->setReadCallback(std::mem_fn(&EventLoop::handleWakeupFd));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() 
{
    assert(quit_);
}

void EventLoop::loop() 
{
    looping_ = true;
    while(!quit_)
    {
      
    }
}

void EventLoop::quit() 
{
  
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
              fmt::ptr(this), threadId_, ::tid());
    abort();
}

void EventLoop::assertInLoopThread() 
{
    if(!isInLoopThread())
      abortNotInLoopThread();
}
