#include "base/Util.h"

namespace ham
{
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
}