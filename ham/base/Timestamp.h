#ifndef HAM_BASE_TIMESTAMP_H
#define HAM_BASE_TIMESTAMP_H

#include "../base/copyable.h"
#include "../base/Types.h"

#include <boost/operators.hpp>

namespace ham
{

///
/// Time stamp in UTC, in microseconds resolution.
///
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp : public ham::copyable,    // 空基类；用于标识该类为可拷贝类型
                  public boost::less_than_comparable<Timestamp>     // 只需要实现<，自动实现>,>=,<=
{
 public:
  ///
  /// Constucts an invalid Timestamp.
  ///
  Timestamp(): microSecondsSinceEpoch_(0)
  {
  }

  ///
  /// Constucts a Timestamp at specific time
  ///
  /// @param microSecondsSinceEpoch
  explicit Timestamp(int64_t microSecondsSinceEpoch);

  // 交换两个时间戳：通过pass by reference交换两者的数据成员
  void swap(Timestamp& that)
  {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }

  // default copy/assignment/dtor are Okay    （不涉及动态内存智能指针等，合成拷贝控制成员可以满足要求）

  string toString() const;
  string toFormattedString() const;

  bool valid() const { return microSecondsSinceEpoch_ > 0; }

  // for internal usage.
  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
  time_t secondsSinceEpoch() const
  { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

  ///
  /// Get time of now.
  ///
  static Timestamp now();
  static Timestamp invalid();

  static const int kMicroSecondsPerSecond = 1000 * 1000;

 private:
  int64_t microSecondsSinceEpoch_;     // 距离1970-01-01 00:00:00
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microseciond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;    // 结果/（1000*1000），返回的是秒数
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}
#endif  // HAM_BASE_TIMESTAMP_H
