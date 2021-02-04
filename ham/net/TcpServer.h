#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__
#include <memory>
#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include "net/Callbacks.h"
namespace ham
{
namespace net
{
class EventLoop;
class Acceptor;
class TcpConnection;
class InetAddress;

class TcpServer : public boost::noncopyable
{
public:
    TcpServer(EventLoop*, const InetAddress&, const std::string& name);
    ~TcpServer();

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    const std::string getName() const { return name_; }
    const std::string getHostPost() const { return hostPost_; } 

private:
    void newConnectionCallback(int connfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> connectionMap;

    EventLoop* loop_;
    bool started_;
    std::unique_ptr<Acceptor> acceptor_;
    connectionMap connections_;

    const std::string name_;
    const std::string hostPost_;

    int nextConnId_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
};


}
}

#endif // __TCPSERVER_H__