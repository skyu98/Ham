#include "net/TcpServer.h"
#include "net/TcpConnection.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketOps.h"
#include "base/Util.h"
#include "net/Acceptor.h"
#include "net/EventLoopThreadPool.h"
#include <assert.h>

namespace ham
{
    namespace net
    {
        TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name) 
            : loop_(loop),
              started_(false),
              name_(name),
              hostPost_(listenAddr.getIpPortStr()),
              nextConnId_(1),
              acceptor_(util::make_unique<Acceptor>(loop, listenAddr)),
              loopThreadPool_(util::make_unique<EventLoopThreadPool>(loop))
        {
            assert(loop);
            acceptor_->setNewConnCallback(
                std::bind(&TcpServer::newConnectionCallback, this, 
                std::placeholders::_1, std::placeholders::_2));
        }
        
        TcpServer::~TcpServer() 
        {
            for(const auto& conn : connections_)
            {
                conn.second->destoryConnection();
            }
        }
        
        void TcpServer::setNumOfThreads(int numOfThreads) 
        {
            assert(numOfThreads >= 0);
            loopThreadPool_->setNumOfThreads(numOfThreads);
        }
        
        void TcpServer::start() 
        {
            if(!started_)
            {
                started_ = true;
            }

            loopThreadPool_->start(threadInitCallback_);

            if(!acceptor_->isListening())
            {
                loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
            }
        }
        
        void TcpServer::newConnectionCallback(int connfd, const InetAddress& peerAddr) 
        {
            loop_->assertInLoopThread();
            EventLoop* nextIoLoop = loopThreadPool_->getNextLoop();
            std::string name = hostPost_ + std::to_string(nextConnId_++);
            InetAddress localAddr (sockets::getLocalAddr(connfd));

            TcpConnectionPtr newTcpConn = std::make_shared<TcpConnection>(
                                            nextIoLoop, name, connfd, localAddr, peerAddr);

            connections_[name] = newTcpConn;
            newTcpConn->setConnectionCallback(connectionCallback_);
            newTcpConn->setMessageCallback(messageCallback_);
            newTcpConn->setCloseCallback(std::bind(&TcpServer::removeConnection,
                                                    this, std::placeholders::_1));
            nextIoLoop->runInLoop(std::bind(&TcpConnection::establishConnection, newTcpConn));
        }
        
        void TcpServer::removeConnection(const TcpConnectionPtr& conn) 
        {
            loop_->assertInLoopThread();
            connections_.erase(conn->getName());

            loop_->queueInLoop(std::bind(&TcpConnection::destoryConnection, conn));
        }
    }
}