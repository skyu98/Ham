#ifndef __CURRENTTHREAD_H__
#define __CURRENTTHREAD_H__

#include "base/Util.h"

namespace ham
{
namespace CurrentThread
{
thread_local static int currentThreadId = 0;

int tid()
{
  if(currentThreadId == 0)
    currentThreadId = util::gettid();
  return currentThreadId;
}
}
}
#endif // __CURRENTTHREAD_H__