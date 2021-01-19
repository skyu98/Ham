#ifndef __BLOCKINGQUEUE_H__
#define __BLOCKINGQUEUE_H__
#include <deque>
#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>

namespace ham
{

template<typename T>
class BlockingQueue : public boost::noncopyable
{
public:
    BlockingQueue() = default;

    void put(const T& task);
    void put(T&&);
    T take()
    size_t size() const;
    

private:
    std::deque<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
};

}


#endif // __BLOCKINGQUEUE_H__