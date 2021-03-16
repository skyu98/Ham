#include "net/TcpClient.h"
#include "net/EventLoop.h"
#include "base/Util.h"
#include "net/Connector.h"
#include "net/SocketOps.h"
namespace ham
{
    namespace net
    {
        TcpClient::TcpClient(EventLoop* loop, 
                    const InetAddress& serverAddr,
                    const std::string& name) 
            : loop_(loop),
              serverAddr_(serverAddr),
              retry_(true),
              connector_(util::make_unique<Connector>(loop, serverAddr))
        {
            connector_->setNewConnectionCallback(
                        std::bind(&TcpClient::newConnection, this,
                        std::placeholders::_1));
        }
        
        TcpClient::~TcpClient() 
        {
            
        }
        
        void TcpClient::connect() 
        {
            connector_->start();
        }
        
        void TcpClient::disconnect() 
        {
            
        }
        
        void TcpClient::reConnect() 
        {
            
        }
        
        void TcpClient::newConnection(int fd) 
        {
            loop_->assertInLoopThread();
            std::string name;
            
            InetAddress localAddr(sockets::getLocalAddr(fd));
            InetAddress peerAddr(sockets::getPeerAddr(fd));
            TcpConnectionPtr conn = std::make_shared<TcpConnection>
                                    (loop_, name, fd, 
                                    localAddr, peerAddr);
                                    
        }
    }
}

    