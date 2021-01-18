#include "../Singleton.h"
#include "../CurrentThread.h"

#include <thread>
#include <string>
#include <boost/noncopyable.hpp>
#include <stdio.h>

class Test : boost::noncopyable
{
 public:
  Test()
  {
    printf("tid=%d, constructing %p\n", ham::CurrentThread::tid(), this);
  }

  ~Test()
  {
    printf("tid=%d, destructing %p %s\n", ham::CurrentThread::tid(), this, name_.c_str());
  }

  const std::string& name() const { return name_; }
  void setName(const std::string& n) { name_ = n; }

 private:
  std::string name_;
};

void threadFunc()
{
  printf("tid=%d, %p name=%s\n",
         ham::CurrentThread::tid(),
         &ham::Singleton<Test>::instance(),
         ham::Singleton<Test>::instance().name().c_str());
  ham::Singleton<Test>::instance().setName("only one, changed");
}

int main()
{
  ham::Singleton<Test>::instance().setName("only one");
  std::thread t1(threadFunc);
  t1.join();
  printf("tid=%d, %p name=%s\n",
         ham::CurrentThread::tid(),
         &ham::Singleton<Test>::instance(),
         ham::Singleton<Test>::instance().name().c_str());
}
