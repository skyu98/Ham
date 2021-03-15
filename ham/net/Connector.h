#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__
#include "net/Socket.h"
#include "net/Channel.h"
#include "net/InetAddress.h"
#include <memory>
#include <functional>
#include <atomic>

class EventLoop;
namespace ham
{
namespace net
{

class Connector
{
public:
    // 不同于Acceptor的cb，connector中对端和本端的ip+port都是确定的，不需要再作为参数传出
    typedef std::function<void(int fd/*, const InetAddress&*/)> newConnectionCallback;
    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    void setNewConnectionCallback(const newConnectionCallback& cb) { cb_ = cb; }

    const InetAddress& serverAddr() const { return serverAddr_; }
private:
    /* data */
    enum State { kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs = 30*1000;			// 30秒，最大重连延迟时间
    static const int kInitRetryDelayMs = 500;				// 0.5秒，初始状态，连接不上，0.5秒后重连

    EventLoop* loop_;
    InetAddress serverAddr_;
    std::atomic<State> state_;
    std::unique_ptr<Channel> connectChannel_;
    std::unique_ptr<Socket> connectSocket_;

    newConnectionCallback cb_;
};
}
}
#endif // __CONNECTOR_H__