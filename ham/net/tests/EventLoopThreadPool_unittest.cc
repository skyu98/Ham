#include "net/EventLoopThreadPool.h"
#include "base/Log.h"
#include "net/TcpServer.h"
#include "net/TcpConnection.h"
#include "net/EventLoop.h"
#include "base/Util.h"
#include "net/InetAddress.h"
#include "net/CurrentThread.h"
#include <memory>

using namespace ham;
using namespace ham::net;

class TestServer
{
public:
    TestServer(EventLoop* loop, const InetAddress& listenAddr, int numOfThreads = 0)
        : baseLoop_(loop),
          server_(util::make_unique<TcpServer>(loop, listenAddr, "Test server"))
    {
        server_->setNumOfThreads(numOfThreads);
        server_->setThreadInitCallback(std::bind(&TestServer::threadInitFunc, this,
                                    std::placeholders::_1));
        server_->setConnectionCallback(std::bind(&TestServer::connectionCallback, this,
                                    std::placeholders::_1));
        server_->setMessageCallback(std::bind(&TestServer::msgCallback, this, 
                                    std::placeholders::_1,      // conn
                                    std::placeholders::_2,      // data
                                    std::placeholders::_3));    // len
    }

    void start()
    {
        server_->start();
    }

private:
    void threadInitFunc(EventLoop* loop)
    {
        std::cout << "thread Init func in thread " << CurrentThread::tid() << std::endl;
    }

    void connectionCallback(const TcpConnectionPtr& conn)
    {
        if(conn->isConnected())
        {
            std::cout << "New conn comes from " << conn->peerAddress().getIpPortStr()
                << " and its name is " << conn->getName() << std::endl;
        }
        else
        {
            std::cout << "Conn " << conn->getName() << " is down..." << std::endl;
        }
    }

    void msgCallback(const TcpConnectionPtr& conn, 
                    const char* data, ssize_t len)
    {
        std::cout << "New message from " << conn->getName()
                << " and its len is " <<  len << std::endl;
    }  

    EventLoop* baseLoop_;
    std::unique_ptr<TcpServer> server_;
};

int main()
{
    if(!Logger::Instance().init("Logger", "./log.txt", Logger::logLevel::trace))
    {
        return -1;
    }

    EventLoop loop;
    InetAddress listenAddr(8888);
    TestServer server(&loop, listenAddr, 4);

    server.start();

    loop.loop();
    return 0;
}