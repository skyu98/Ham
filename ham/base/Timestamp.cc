#include "../base/Timestamp.h"

#include <chrono>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

using namespace ham;
using namespace std::chrono;

static_assert(sizeof(Timestamp) == sizeof(int64_t), "Size of Timestamp should be that of int64_t");  // 编译时断言（assert是运行时断言）

Timestamp::Timestamp(int64_t microseconds)
  : microSecondsSinceEpoch_(microseconds)
{
}

string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);  // PRId64；用于跨平台格式化输出int_64
  return buf;
}

string Timestamp::toFormattedString() const
{
  char buf[32] = {0};
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
  
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);   //_r代表线程安全的

  snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
      microseconds);
  return buf;
}

Timestamp Timestamp::now()
{
    // system_clock::now() 获取当前time_point
    // time_since_epoch() 获取当前到1970-1-1 00:00:00 的duration，转换为microseconds    
    // count()返回微秒数
    int64_t time_since_epoch = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return Timestamp(time_since_epoch);
}

Timestamp Timestamp::invalid()
{
  return Timestamp();
}


