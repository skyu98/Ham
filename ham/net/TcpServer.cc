#include "net/TcpServer.h"
#include "net/TcpConnection.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketOps.h"
#include "base/Util.h"
#include "net/Acceptor.h"
#include <functional>
namespace ham
{
    namespace net
    {
        TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name) 
            : loop_(loop),
              hostPost_(listenAddr.getIpPortStr()),
              name_(name),
              acceptor_(util::make_unique<Acceptor>(loop, listenAddr)),
              nextConnId_(1)
        {
            acceptor_->setNewConnCallback(
                std::bind(&TcpServer::newConnectionCallback, this, 
                std::placeholders::_1, std::placeholders::_2));
        }
        
        TcpServer::~TcpServer() 
        {
            
        }
        
        void TcpServer::start() 
        {
            
        }
        
        void TcpServer::newConnectionCallback(int connfd, const InetAddress& peerAddr) 
        {
            loop_->assertInLoopThread();
            std::string name = hostPost_ + std::to_string(nextConnId_);
            InetAddress localAddr (sockets::getLocalAddr(connfd));

            TcpConnectionPtr newTcpConn = std::make_shared<TcpConnection>(
                                            loop_, name, connfd, localAddr);
        }
    }
}