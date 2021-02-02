#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__
#include <functional>
#include <memory>


namespace ham
{
namespace net
{
    // All client visible callbacks go here.
    typedef std::function<void()> TimerCallback;

    class TcpConnection;
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
    typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
    typedef std::function<void (const TcpConnectionPtr&,
                                const char* data,
                                ssize_t len)> MessageCallback;
}
}

#endif // __CALLBACKS_H__