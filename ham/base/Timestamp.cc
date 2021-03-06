#include "Timestamp.h"

using namespace std::chrono;

namespace ham
{   static_assert(sizeof(Timestamp) == sizeof(int64_t), 
            "Size of Timestamp should be that of int64_t");  // 编译时断言（assert是运行时断言）
    
    Timestamp& Timestamp::swap(Timestamp& src) 
    {
        std::swap(microsecondsFromEpoch_, src.microsecondsFromEpoch_);
        return *this;
    }

    std::string Timestamp::toString() const
    {
        return std::to_string(microsecondsFromEpoch_ / kMicrosecondsPerSecond)
            + "." + std::to_string(microsecondsFromEpoch_ % kMicrosecondsPerSecond);
    }
    
    std::string Timestamp::toFormatTime() const
    {
        std::time_t time = microsecondsFromEpoch_ / kMicrosecondsPerSecond;  // ms --> s
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
        return ss.str();
    }
    
    Timestamp Timestamp::now() 
    {
        int64_t nowFromEpoch = duration_cast<microseconds>
                                (system_clock::now().time_since_epoch()).count();
        return Timestamp(nowFromEpoch);
    }
    
    Timestamp Timestamp::invalid() 
    {
        return Timestamp();
    }
    
}