#include "net/TcpConnection.h"
#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "base/Log.h"
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
        
        void TcpConnection::establishConnection() 
        {
            loop_->assertInLoopThread();
            assert(state_ = State::kConnecting);
            setState(State::kConnected);

            channel_->tie(shared_from_this());
            channel_->enableReading();
            connectionCallback_(shared_from_this());
        }
        
        void TcpConnection::destoryConnection() 
        {
            
        }
        
        void TcpConnection::handleRead(Timestamp receiveTime) 
        {
            loop_->assertInLoopThread();
            int savedErrno = 0;
            // 从conn_fd读取数据
            char buf[65536];
            ssize_t n = ::read(channel_->getFd(), buf, sizeof buf);
            if (n > 0)
            {
                messageCallback_(shared_from_this(), buf, n);
            }
            else if (n == 0)
            {
                handleClose();
            }
            else
            {
                errno = savedErrno;
                ERROR("TcpConnection::handleRead");
                handleError();
            }
        }
        
        void TcpConnection::handleClose() 
        {
            
        }
        
        void TcpConnection::handleError() 
        {
            int err = sockets::getSocketError(channel_->getFd());
            ERROR("TcpConnection::handleError [{}] - SO_ERROR = {} {}", name_, err, strerror_tl(err));
        }
    }
}