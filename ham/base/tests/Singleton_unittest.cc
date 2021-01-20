#include "../Singleton.h"
#include <thread>
#include <string>
#include <iostream>

using namespace ham;

class Test
{
public:
    Test()
        : name_("init")
    {
        std::cout << "Test ctr in thread " << std::this_thread::get_id() << std::endl;
    }

    ~Test()
    {
        std::cout << "Test dtr in thread " << std::this_thread::get_id() << std::endl;
    }

    void setName(const std::string& name)
    {
        name_ = name;
    }

    const std::string& getName() const
    {
        return name_;
    }

private:
    std::string name_;
};

void threadFunc(const std::string& name)
{
    std::cout << "I have a singleton named \""
            << SingletonPerThread<Test>::getInstance().getName().c_str()
            << "\" at " << &(SingletonPerThread<Test>::getInstance())
            <<" in thread " << std::this_thread::get_id() << std::endl;
    
    SingletonPerThread<Test>::getInstance().setName(name);

    std::cout << "now I change its name to \""
            << SingletonPerThread<Test>::getInstance().getName().c_str()
            << "\" and it's still at " << &(SingletonPerThread<Test>::getInstance())
            << " in thread " << std::this_thread::get_id() << std::endl;
}

int main()
{
    SingletonPerThread<Test>::getInstance().setName("main one");
    std::cout << "I have a singleton named \""
            << SingletonPerThread<Test>::getInstance().getName().c_str()
            << "\" at " << &(SingletonPerThread<Test>::getInstance())
            << " in thread " << std::this_thread::get_id() << std::endl;
    
    std::thread t1(threadFunc, "thread 1");
    std::thread t2(threadFunc, "thread 2");

    t1.join();

    std::cout << "I have a singleton named \""
            << SingletonPerThread<Test>::getInstance().getName().c_str()
            << "\" at " << &(SingletonPerThread<Test>::getInstance())
            << " in thread " << std::this_thread::get_id() << std::endl;

    t2.join();
    return 0;
}