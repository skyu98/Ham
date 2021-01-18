#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <boost/noncopyable.hpp>

namespace ham
{

template<typename T>
class Singleton : boost::noncopyable
{
public:
    static T& instance()
    {
        static T instance_;
        return instance_;
    }

private:
    Singleton(){};
    ~Singleton(){};
};

}



#endif // __SINGLETON_H__