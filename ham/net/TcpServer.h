#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__
#include <memory>
#include <map>
#include <string>
#include <functional>
#include <boost/noncopyable.hpp>
#include "net/Callbacks.h"
#include "net/TcpConnection.h"
namespace ham
{
namespace net
{
class EventLoop;
class Acceptor;
class TcpConnection;
class InetAddress;
class EventLoopThreadPool;

class TcpServer : public boost::noncopyable
{
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
public:
    TcpServer(EventLoop*, const InetAddress&, const std::string& name);
    ~TcpServer();
    
    void setNumOfThreads(int numOfThreads);
    void start();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompeleteCallback(const WriteCompeleteCallback& cb) { writeCompeleteCallback_ = cb; }
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }

    const std::string getName() const { return name_; }
    const std::string getHostPost() const { return hostPost_; } 

private:
    void newConnectionCallback(int connfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> connectionMap;

    EventLoop* loop_;
    bool started_;
    const std::string name_;
    const std::string hostPost_;
    int nextConnId_;

    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> loopThreadPool_;
    connectionMap connections_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompeleteCallback writeCompeleteCallback_;
    ThreadInitCallback threadInitCallback_;
};


}
}

#endif // __TCPSERVER_H__