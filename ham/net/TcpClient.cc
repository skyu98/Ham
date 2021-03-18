#include "net/TcpClient.h"
#include "net/EventLoop.h"
#include "base/Util.h"
#include "net/Connector.h"
#include "net/SocketOps.h"
#include <assert.h>
namespace ham
{
    namespace net
    {
        namespace detail
        {
            void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn) 
            {
                loop->queueInLoop(std::bind(&TcpConnection::destoryConnection, conn));
            }

            void removeConnector(const std::unique_ptr<Connector>& connector) 
            {
                //connector->
            }
        }


        TcpClient::TcpClient(EventLoop* loop, 
                    const InetAddress& serverAddr,
                    const std::string& name) 
            : loop_(loop),
              serverAddr_(serverAddr),
              retry_(false),
              connect_(false),
              name_(name),
              nextConnId_(1),
              connector_(util::make_unique<Connector>(loop, serverAddr))
        {
            connector_->setNewConnectionCallback(
                        std::bind(&TcpClient::newConnection, this,
                        std::placeholders::_1));
            INFO("TcpClient::TcpClient[{}] - connector {}", name_, fmt::ptr(connector_));
        }
        
        TcpClient::~TcpClient() 
        {
            INFO("TcpClient::~TcpClient[{}] - connector {}", name_, fmt::ptr(connector_));
            bool unique = establishedConnection_.unique();
            // 此时若已经建立连接
            if(establishedConnection_)
            {   
                CloseCallback cb = std::bind(detail::removeConnection, loop_, std::placeholders::_1);
                establishedConnection_->setCloseCallback(cb);
                if(unique)  // 如果不存在queueInLoop的TcpConnection::destoryConnection()
                {
                    // 那么需要妥善处理conn，不能让其自动析构；
                    // forceClose()中会调用closeCallback(),所以在此之前需要重新设置closeCallback，使其不再回调
                    establishedConnection_->forceClose();  
                }
            }
            else
            {
                // conn处于未连接状态，将connetor_停止即可
                connector_->stop();
                // FIXME: HACK
                // bind不管函数形参是否为引用，都会进行复制；unique_ptr不能复制，所以使用ref()
                loop_->runAfter(1, std::bind(&detail::removeConnector, std::ref(connector_)));
            }
        }
        
        // 发起连接
        void TcpClient::connect() 
        {
            connect_ = true;
            connector_->start();
        }
        
        // 已经建立连接时，关闭连接
        void TcpClient::disconnect() 
        {
            connect_ = false;
            if(establishedConnection_)
            {
                establishedConnection_->shutdown();
            } 
        }
        
        // 未建立连接时，停止connector
        void TcpClient::stop() 
        {
            connect_ = false;
            connector_->stop();
        }

        void TcpClient::newConnection(int fd) 
        {
            loop_->assertInLoopThread();
            InetAddress peerAddr(sockets::getPeerAddr(fd));
            std::string name(name_ + ":" + peerAddr.getIpPortStr() + "#" + std::to_string(nextConnId_++));
            InetAddress localAddr(sockets::getLocalAddr(fd));
            
            TcpConnectionPtr conn = std::make_shared<TcpConnection>
                                    (loop_, name, fd, 
                                    localAddr, peerAddr);
            
            conn->setConnectionCallback(connectionCallback_);
            conn->setMessageCallback(messageCallback_);
            conn->setWriteCompleteCallback(writeCompleteCallback_);
            conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
            conn->establishConnection();
            establishedConnection_ = conn;
        }
        
        void TcpClient::removeConnection(const TcpConnectionPtr& conn) 
        {
            loop_->assertInLoopThread();
            assert(loop_ == conn->getOwnnerLoop());

            establishedConnection_.reset();
            loop_->queueInLoop(std::bind(&TcpConnection::destoryConnection, conn));

            if(retry_ && connect_)  // 未主动断开连接并且指定断开后重连
            {
                INFO("TcpClient::connect[{}] - Reconnecting to {}", 
                    name_, connector_->serverAddr().getIpPortStr());
                connector_->restart();
            }
        }
    }
}

    