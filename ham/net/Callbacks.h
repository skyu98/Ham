#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__
#include <functional>


namespace ham
{
namespace net
{
    // All client visible callbacks go here.
    typedef std::function<void()> TimerCallback;
}
}

#endif // __CALLBACKS_H__