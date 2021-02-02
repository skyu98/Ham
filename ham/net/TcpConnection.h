#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__
#include <boost/noncopyable.hpp>
#include <memory>   
#include <string>

#include "net/InetAddress.h"
#include "net/Callbacks.h"

namespace ham
{
namespace net
{

class EventLoop;
class Channel;
class Socket;

class TcpConnection : public boost::noncopyable,
                             std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, const std::string& name, 
                    InetAddress localAddr, InetAddress peerAddr);
    ~TcpConnection();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
private:
    enum State
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting,
        num_state
    };

    EventLoop* loop_;
    std::string name_;
    State state_;

    InetAddress localAddr_;
    InetAddress peerAddr_;

    std::shared_ptr<Socket> socket_;
    std::shared_ptr<Channel> channel_;

    ConnectionCallback connectionCallback_;
    MessageCallback messagrCallback_;
    CloseCallback closeCallback_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}

#endif // __TCPCONNECTION_H__