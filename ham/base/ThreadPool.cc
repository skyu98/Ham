#include "ThreadPool.h"
#include <iostream>
namespace ham
{
    ThreadPool::ThreadPool(const std::string& name)
        : isRunning_(false),
          name_(name),
          maxsize_(0)
    {
    }
    
    ThreadPool::~ThreadPool() 
    {
        // std::cout << "size is " << queue_.size() << std::endl;
        if(isRunning_)
            stop();
    }
    
    void ThreadPool::start(int numOfThreads) 
    {
        if(!isRunning_)
        {
            threads_.reserve(numOfThreads);
            isRunning_ = true;
            for(int i = 0;i < numOfThreads;++i)
            {
                threads_.emplace_back(std::thread(&ThreadPool::threadFunc, this));
            }
        }
        if(numOfThreads == 0 && cb_)
            cb_();
    }
    
    void ThreadPool::stop() 
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            isRunning_ = false;
            // 不管在等待什么，全部通知；避免上锁检查状态
            notEmpty_.notify_all();
            notFull_.notify_all();
        }
        
        for(auto& t : threads_)
        {
            t.join();  // 收拾各个线程的残局；还没做完，就等线程做完任务
        }
    }
    
    void ThreadPool::run(const Task& newTask) 
    {
        if(threads_.empty())
            newTask();
        else
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(isFull() && isRunning_)
            {
                notFull_.wait(lock);
            }
            queue_.emplace_back(newTask);
            std::cout << "size is " << queue_.size() << std::endl;
            notEmpty_.notify_one();
        }
    }
    
    void ThreadPool::run(Task&& newTask) 
    {
        if(threads_.empty())
            newTask();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(isFull() && isRunning_)
            {
                notFull_.wait(lock);
            }
            queue_.emplace_back(std::move(newTask));
        }
        notEmpty_.notify_one();
    }
    
    ThreadPool::Task ThreadPool::take() 
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(queue_.empty() && isRunning_)
        {
            notEmpty_.wait(lock);
        }
        Task task;
        if(!queue_.empty())
        {
            task = std::move(queue_.front());
            queue_.pop_front();
            if(maxsize_ > 0)
                notFull_.notify_one();
        }
        return task;
    }
    
    void ThreadPool::threadFunc() 
    {
       try
        { 
            if(cb_)
                cb_();
            while(isRunning_)
            {
                Task task(take());
                if(task)
                    task();
            }
        }
        catch(const Exception& ex)
        {
            std::cerr << "reasons: " << std::string(ex.what()) << std::endl;
            std::cerr << "stack trace: \n" << std::string(ex.stackTrace()) << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "unknown error has occured..." << std::endl;
        }
       
    }
    
    bool ThreadPool::isFull() const
    {
        return maxsize_ > 0 && queue_.size() >= maxsize_;
    }
}