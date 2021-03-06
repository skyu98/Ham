#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "net/Channel.h"
#include "net/TimerId.h"
#include "net/InetAddress.h"
#include <boost/noncopyable.hpp>
#include <memory>
#include <functional>
#include <atomic>


class EventLoop;
namespace ham
{
namespace net
{

class Connector : public boost::noncopyable, 
                  public std::enable_shared_from_this<Connector>
{
public:
    // 不同于Acceptor的cb，connector中对端和本端的ip+port都是确定的，不需要再作为参数传出
    typedef std::function<void(int fd/*, const InetAddress&*/)> newConnectionCallback;
    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    void setNewConnectionCallback(const newConnectionCallback& cb) { newConnCb_ = cb; }

    const InetAddress& serverAddr() const { return serverAddr_; }
private:
    void startInLoop();
    void stopInLoop();
    void connect();
    void establishConnection(int fd);
    void retry(int fd);

    void handleWrite();
    void handleRead();
    void handleError();

    int removeAndResetChannel();
    void resetChannel();

    /* data */
    enum State { kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs = 30 * 1000;         // 30秒，最大重连延迟时间	
    static const int kInitRetryDelayMs = 5000;				// 0.5秒，初始状态，连接不上，0.5秒后重连	

    EventLoop* loop_;
    InetAddress serverAddr_;
    std::atomic<State> state_;
    std::unique_ptr<Channel> connectChannel_;
    newConnectionCallback newConnCb_;
    std::atomic_bool connect_;
    int retryDelayMs_;		// 重连延迟时间（单位：毫秒）
    TimerId retryTimerId_;
    bool validTimer_;
};

}
}
#endif // __CONNECTOR_H__