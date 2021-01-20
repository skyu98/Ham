#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__
#include <thread>
#include <vector>
#include <deque>
#include <mutex>
#include <string>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <boost/noncopyable.hpp>

#include "Exception.h"


namespace ham
{
class ThreadPool : public boost::noncopyable
{
    typedef std::function<void()> Task;
    typedef std::function<void()> threadInitCallback;

public:
    ThreadPool(const std::string& name, size_t maxsize = 0);
    ~ThreadPool();

    void start(int numOfThreads);
    void stop();

    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_ ;}

    void run(const Task& newTask);
    void run(Task&& newTask);

    void setMaxQueueSize(size_t size) { maxsize_ = size; }
    void setInitCallback(const threadInitCallback& cb) { cb_ = cb; }
    size_t queueSize() const;

private:
    Task& take();
    void threadFunc();
    bool isFull();

    bool isRunning_;

    std::vector<std::thread> threads_;
    std::deque<Task> queue_;

    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;

    std::string name_;
    size_t maxsize_;
    threadInitCallback cb_;
};

}

#endif // __THREADPOOL_H__