#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__
#include <boost/noncopyable.hpp>
#include <memory>   
#include <string>
#include <atomic>

#include "net/InetAddress.h"
#include "net/Callbacks.h"
#include "base/Timestamp.h"

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
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                    InetAddress localAddr, InetAddress peerAddr);
    ~TcpConnection();

    const std::string getName() const { return name_; } 

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void establishConnection();
    void destoryConnection();
private:
    enum State
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting,
        num_state
    };

    void setState(State state) { state_ = state;}
    void handleRead(Timestamp receiveTime);
    void handleClose();
    void handleError();

    EventLoop* loop_;
    std::string name_;
    std::atomic<State> state_;

    InetAddress localAddr_;
    InetAddress peerAddr_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}

#endif // __TCPCONNECTION_H__