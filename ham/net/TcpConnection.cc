#include "net/TcpConnection.h"
#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "base/Log.h"
#include "base/Util.h"
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
              localAddr_(localAddr),
              peerAddr_(peerAddr),
              socket_(util::make_unique<Socket>(sockfd)),
              channel_(util::make_unique<Channel>(loop, sockfd))
        {
            channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this,
                                    std::placeholders::_1));
            channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
            channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
            DEBUG("TcpConnection::ctor[{}] at {} fd= {} ", name_, fmt::ptr(this), sockfd);
            socket_->setKeepAlive(true);
        }
        
        TcpConnection::~TcpConnection() 
        {
            DEBUG("TcpConnection::ctor[{}] at {} fd= {} ", name_, fmt::ptr(this), channel_->getFd());
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
            loop_->assertInLoopThread();
            // 如果是TcpServer主动关闭连接，则不会经过handleClose，需要设置状态
            if (state_ == kConnected)
            {
                setState(kDisconnected);
                channel_->disableAll();

                connectionCallback_(shared_from_this());
            }
            channel_->remove();
        }
        
        void TcpConnection::send(const std::string& msg) 
        {
            if(state_ == kConnected)
            {
                if(loop_->isInLoopThread())
                {
                    sendInLoop(msg);
                }
                else
                {
                    void (TcpConnection::* funcPtr)(const std::string&) = &TcpConnection::sendInLoop;
                    loop_->runInLoop(std::bind(funcPtr, this, msg));
                    // loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, msg));
                    // 此处不能直接bind，因为该函数是private的；只能获取其地址然后bind
                }

            }
        }
        
        void TcpConnection::send(Buffer& buffer) 
        {
             if(state_ == kConnected)
            {
                if(loop_->isInLoopThread())
                {
                    sendInLoop(buffer.peek(), buffer.readableBytes());
                    buffer.retrieveAll();
                }
                else
                {
                    // https://stackoverflow.com/questions/6538133/how-can-boostbind-call-private-methods
                    void (TcpConnection::* funcPtr)(const std::string&) = &TcpConnection::sendInLoop;
                    loop_->runInLoop(std::bind(funcPtr, this, buffer.retrieveAllAsString()));
                }

            }
        }
        
        void TcpConnection::shutdown() 
        {
            if(state_ == kConnected)
            {
                // 这里只改变状态
                setState(kDisconnecting);
                loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
            }
        }
        
        void TcpConnection::handleRead(Timestamp receiveTime) 
        {
            loop_->assertInLoopThread();
            int savedErrno = 0;
            // 从conn_fd读取数据
            ssize_t n = inputBuffer_.readFd(channel_->getFd(), &savedErrno);
            if (n > 0)
            {
                messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
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
            loop_->assertInLoopThread();
            setState(State::kDisconnected);
            channel_->disableAll();

            closeCallback_(shared_from_this());
        }
        
        void TcpConnection::handleError() 
        {
            int err = sockets::getSocketError(channel_->getFd());
            ERROR("TcpConnection::handleError [{}] - SO_ERROR = {} {}", name_, err, strerror_tl(err));
        }
        
        void TcpConnection::sendInLoop(const std::string& msg) 
        {
            sendInLoop(msg.c_str(), msg.size());
        }
        
        void TcpConnection::sendInLoop(const void* data, size_t len) 
        {
            loop_->assertInLoopThread();
            ssize_t n = sockets::write(channel_->getFd(), data, len);
        }
        
        void TcpConnection::shutdownInLoop() 
        {
            loop_->assertInLoopThread();
            if(!channel_->isWriting())
            {
                setState(kDisconnected);
                socket_->shutdownWrite();
            }
        }
    }
}