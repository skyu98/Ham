#ifndef __SINGLETON_H__
#define __SINGLETON_H__
#include <string>

namespace ham
{

template<typename T>
class Singleton 
{
public:
    Singleton() = delete;
    ~Singleton() = default;

    static T& getInstance()
    {
        static T obj;
        return obj;
    }

    const std::string& getName() const
    {
        return name_;
    }

    void setName_(const std::string& newName)
    {
        name_ = newName;
    }


private:
    std::string name_;
};

template<typename T>
class SingletonPerThread
{
public:
    SingletonPerThread() = delete;
    ~SingletonPerThread() = default;

    static T& getInstance()
    {
        static thread_local T obj;
        return obj;
    }
};

}


#endif // __SINGLETON_H__