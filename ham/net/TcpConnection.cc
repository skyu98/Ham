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
              channel_(util::make_unique<Channel>(loop, sockfd)),
              highWaterMark_(64*1024*1024) // 64MiB
        {
            channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this,
                                    std::placeholders::_1));
            channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
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
        
        void TcpConnection::handleWrite() 
        {
            loop_->assertInLoopThread();
            // 此时按理说应该是在关注可读事件
            if(channel_->isWriting())
            {
                ssize_t n_wrote = sockets::write(channel_->getFd(),
                                                outputBuffer_.peek(),
                                                outputBuffer_.readableBytes());
                if(n_wrote > 0)
                {
                    outputBuffer_.retrieve(n_wrote);
                    if(outputBuffer_.readableBytes() == 0)// 数据全部写完
                    {
                        channel_->disableWriting();  // 立即取消关注可写，否则busy loop
                        if(writeCompeleteCallback_)
                        {
                            loop_->queueInLoop(
                                std::bind(&TcpConnection::writeCompeleteCallback_, 
                                shared_from_this()));
                        }
                    }
                }
                else
                {
                    ERROR("TcpConnection::handleWrite() Error.");
                }
            }
            else // 没有关注EPOLLOUT事件
            {  
            TRACE("Connection fd = {} is down, no more writing", channel_->getFd());
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
            size_t remaining = len; // 剩余的数据长度
            ssize_t n_wrote = 0;
            bool faultError = false;

            // 当outputBuf当中没数据（此时肯定也没关注可写事件），直接write
            if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
            {
                n_wrote = sockets::write(channel_->getFd(), data, len);
                if(n_wrote >= 0)
                {
                    remaining -= n_wrote;
                    if(remaining == 0 && writeCompeleteCallback_)
                    {
                        loop_->queueInLoop(
                            std::bind(TcpConnection::writeCompeleteCallback_, 
                            shared_from_this()));
                        // 此处如果是runInLoop，那么如果writeCompeleteCallback_有send操作，
                        // 就会一直递归
                    }
                }
                else // n_wrote < 0
                {
                    n_wrote = 0; // 实际发送大小为0
                    if(errno != EWOULDBLOCK) // 不是因为发送缓存区满了
                    {
                        ERROR("TcpConnection::sendInLoop() Error");
                        if (errno == EPIPE || errno == ECONNRESET)// FIXME: any others? 
                        {
                            faultError = true;
                        }
                    }
                }

                assert(remaining <= len);
                // 如果还没写完且没有错误发生（即发送缓冲区满），剩下的数据需要写到outputBuf
                if(!faultError && remaining > 0)
                {
                    size_t old_len = outputBuffer_.readableBytes();
                    if(old_len < highWaterMark_ 
                    && old_len + remaining >= highWaterMark_
                    && highWaterMarkCallback_)
                    {
                        loop_->queueInLoop(
                            std::bind(TcpConnection::highWaterMarkCallback_,
                            shared_from_this(), old_len + remaining));
                    }
                    outputBuffer_.append(static_cast<const char*>(data) + n_wrote, remaining);
                }

                // 关注可写事件，可写时将剩余数据发出
                if(!channel_->isWriting())
                {
                    channel_->enableWriting();
                }
            }
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