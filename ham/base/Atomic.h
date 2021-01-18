#ifndef HAM_BASE_ATOMIC_H
#define HAM_BASE_ATOMIC_H

#include <atomic>

namespace ham
{
    typedef std::atomic<int32_t> AtomicInt32;
    typedef std::atomic<int64_t> AtomicInt64;
}

#endif