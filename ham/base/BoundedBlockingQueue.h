#ifndef __BOUNDEDBoundedBlockingQueue_H__
#define __BOUNDEDBoundedBlockingQueue_H__
#include <deque>
#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>

namespace ham
{

template<typename T>
class BoundedBlockingQueue : public boost::noncopyable
{
public:
    BoundedBlockingQueue() = default;

    void put(const T& task);
    void put(T&& task);

    T take();
    size_t size() const;
    size_t capacity() const;
    bool full() const;
    bool empty() const;
    

private:
    std::deque<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    boost::circular_buffer<T>  queue_;         // 环形队列，重复利用内存。（这里内部用指针实现的）
};

}


#endif // __BOUNDEDBoundedBlockingQueue_H__