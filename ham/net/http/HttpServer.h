#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__
#include "net/TcpServer.h"

namespace ham
{
namespace net
{
class HttpRequest;
class HttpResponse;
class HttpServer
{
public:
    typedef std::function<void (const HttpRequest&, 
                                    HttpResponse&)> HttpCallback;
    HttpServer(EventLoop* loop, 
                const InetAddress& listenAddr,
                const std::string name);

    const EventLoop* getLoop() const { return server_.getLoop(); }

    /// Not thread safe, callback be registered before calling start().
    void setHttpCallback(const HttpCallback& cb)
    {
        httpCallback_ = cb;
    }

    void setThreadNum(int numThreads)
    {
        server_.setNumOfThreads(numThreads);
    }

    void start();
    
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer&, Timestamp);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest&);
    
    EventLoop* loop_;
    TcpServer server_;
    HttpCallback httpCallback_;
};
}
}

#endif // __HTTPSERVER_H__