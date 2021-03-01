#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__
#include <functional>
#include <memory>
#include "base/Timestamp.h"


namespace ham
{
namespace net
{
    class Buffer;
    // All client visible callbacks go here.
    typedef std::function<void()> TimerCallback;

    class TcpConnection;
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
    typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
    typedef std::function<void (const TcpConnectionPtr&,
                                Buffer&,Timestamp)> MessageCallback;
    typedef std::function<void (const TcpConnectionPtr&)> WriteCompeleteCallback;
    typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
    // typedef std::function<void (const TcpConnectionPtr&,
    //                             const char* data,
    //                             ssize_t len)> MessageCallback;
}
}

#endif // __CALLBACKS_H__