#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include "copyable.h"
#include "Types.h"
#include <string>
#include <chrono>
#include <boost/operators.hpp>

namespace ham
{

class Timestamp:  public ham::copyable,
                  public boost::less_than_comparable<Timestamp>
{
public:
    Timestamp(int64_t microsecondsFromEpoch = 0)
            : microsecondsFromEpoch_(microsecondsFromEpoch)
    {}
    
    ~Timestamp(){};

    // defalut assignment & copy ctr is ok
    
    std::string toString() const;

    Timestamp& swap(Timestamp& src);
    
    bool valid() const { return microsecondsFromEpoch_ > 0; }

    int64_t microsecondsFromEpoch() const { return microsecondsFromEpoch_; }
    int64_t secondsFromEpoch() const { return microsecondsFromEpoch_ / kMicrosecondsPerSecond; }
    
    // 以下函数与this指针无关，所以是静态成员函数
    static Timestamp now(); 
    static Timestamp invalid();

    static const int kMicrosecondsPerSecond = 1000 * 1000;

private:
    int64_t microsecondsFromEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)  // int64传值比传地址快
{
    return lhs.microsecondsFromEpoch() < rhs.microsecondsFromEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)  
{
    return lhs.microsecondsFromEpoch() == rhs.microsecondsFromEpoch();
}

inline double timeDifference(Timestamp lhs, Timestamp rhs)
{
    int64_t diff = lhs.microsecondsFromEpoch() - rhs.microsecondsFromEpoch();
    diff /= Timestamp::kMicrosecondsPerSecond;
    return static_cast<double>(diff);
}

inline Timestamp addTime(Timestamp src, double secondsToBeAdded)
{
    int64_t microseconds = static_cast<int64_t>(secondsToBeAdded * 
                                                Timestamp::kMicrosecondsPerSecond);
    return Timestamp(src.microsecondsFromEpoch() + microseconds);
}

}

#endif // __TIMESTAMP_H__