#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__
#include <memory>
#include "net/Callbacks.h"
#include "TcpConnection.h"
#include <functional>
#include <atomic>
#include <string>
namespace ham
{
namespace net
{
class EventLoop;
class Connector;
class TcpClient
{
public:
    TcpClient(EventLoop* loop, 
            const InetAddress& serverAddr,
            const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    
    void setRetry(bool on) { retry_ = on; }
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    
    TcpConnectionPtr getConnection () const { return establishedConnection_; }

private:
    void reConnect();
    void newConnection(int fd);

private:
    /* data */
    EventLoop* loop_;
    InetAddress serverAddr_;
    std::atomic_bool retry_;
    std::unique_ptr<Connector> connector_;
    TcpConnectionPtr establishedConnection_;

    ConnectionCallback connectionCallback_;		// 连接建立回调函数
    MessageCallback messageCallback_;				// 消息到来回调函数
    WriteCompleteCallback writeCompleteCallback_;	// 数据发送完毕回调函数
};

}
}
#endif // __TCPCLIENT_H__