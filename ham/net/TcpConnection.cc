#include "net/TcpConnection.h"
#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include <functional>
namespace ham
{
    namespace net
    {
        TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                                    InetAddress localAddr, InetAddress peerAddr)
            : loop_(loop),
              name_(name),
              state_(State::kConnecting),
              localAddr_(localAddr_),
              peerAddr_(peerAddr),
              socket_(std::make_shared<Socket>(sockfd)),
              channel_(std::make_shared<Channel>(loop, sockfd))
        {
            channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this,
                                    std::placeholders::_1));
            channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
            channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
            DEBUG("TcpConnection::ctor[{}] at {} fd= {} ", name_, this, sockfd);
            socket_->setKeepAlive(true);
        }
        
        TcpConnection::~TcpConnection() 
        {
            DEBUG("TcpConnection::ctor[{}] at {} fd= {} ", name_, this, channel_->getFd());
        }
        
        void TcpConnection::handleRead(Timestamp receiveTime) 
        {
            
        }
        
        void TcpConnection::handleClose() 
        {
            
        }
        
        void TcpConnection::handleError() 
        {
            
        }
    }
}