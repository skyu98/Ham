#ifndef __CURRENTTHREAD_H__
#define __CURRENTTHREAD_H__

#include "base/Util.h"

namespace ham
{
namespace CurrentThread
{
extern thread_local pid_t currentThreadId;  // EventLoop.cc

inline pid_t tid()
{
  if(currentThreadId == 0)
    currentThreadId = util::gettid();
  return currentThreadId;
}
}
}
#endif // __CURRENTTHREAD_H__