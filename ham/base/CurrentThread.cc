#include "../base/CurrentThread.h"
#include "../base/Exception.h"

#include <type_traits>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>


namespace ham
{
namespace CurrentThread
{
  // __thread修饰的变量是线程局部存储的。
  __thread int t_cachedTid = 0;		// 线程真实pid（tid）的缓存，
									// 是为了减少::syscall(SYS_gettid)系统调用的次数
									// 提高获取tid的效率
  __thread char t_tidString[32];	// 这是tid的字符串表示形式
  __thread const char* t_threadName = "unknown";
  const bool sameType = std::is_same<int, pid_t>::value;   // 内部完全一样，类名不一样，也不是same
  static_assert(sameType == true, "pid_t is not int here!!!");
}

namespace detail
{

pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    ham::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
  }
};

ThreadNameInitializer init;
}
}

using namespace ham;

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    int n = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    assert(n == 6); (void) n;
  }
}

bool CurrentThread::isMainThread()
{
  return tid() == ::getpid();
}
