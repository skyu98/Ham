#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include "base/Timestamp.h"
#include <memory>
#include <functional>

namespace ham
{
namespace net
{
class EventLoop;
class Channel;
class Socket;
class InetAddress;

class Acceptor
{
public:
    typedef std::function<void(int sockfd, const InetAddress&)> newConnCallback;
    Acceptor(EventLoop* loop, const InetAddress& addr, bool reusePort = true);
    ~Acceptor();

    void listen();
    bool isListening() const { return listening_; }

    void setNewConnCallback(const newConnCallback& cb) { newConnCallback_ = cb;}
     
private:
    void handleRead(Timestamp time);
    EventLoop* loop_;
    bool listening_;
    int idleFd_;

    std::unique_ptr<Socket> acceptSocket_;
    std::unique_ptr<Channel>  acceptChannel_;
    
    newConnCallback newConnCallback_;
};
}
}
#endif // __ACCEPTOR_H__