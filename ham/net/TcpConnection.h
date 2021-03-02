#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__
#include <boost/noncopyable.hpp>
#include <memory>   
#include <string>
#include <atomic>
#include "net/Socket.h"
#include "net/InetAddress.h"
#include "net/Buffer.h"
#include "net/Callbacks.h"
#include "base/Timestamp.h"

namespace ham
{
namespace net
{

class EventLoop;
class Channel;
class Socket;

class TcpConnection :   boost::noncopyable,
                        public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                    InetAddress localAddr, InetAddress peerAddr);
    ~TcpConnection();

    const std::string getName() const { return name_; } 
    EventLoop* getOwnnerLoop() const { return loop_; }
    const InetAddress& localAddress() { return localAddr_; }
    const InetAddress& peerAddress() { return peerAddr_; }

    bool isConnected() const { return state_ == State::kConnected; }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { highWaterMarkCallback_ = cb; }
    void setWriteCompeleteCallback(const WriteCompeleteCallback& cb) { writeCompeleteCallback_ = cb; }

    void establishConnection();
    void destoryConnection();

    void send(const std::string& msg);
    void send(Buffer& buffer);

    void setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on);}
    void shutdown();

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
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& msg);
    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();

    EventLoop* loop_;
    std::string name_;
    std::atomic<State> state_;

    InetAddress localAddr_;
    InetAddress peerAddr_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    size_t highWaterMark_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompeleteCallback writeCompeleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}

#endif // __TCPCONNECTION_H__