#include "net/Connector.h"
#include "base/Util.h"
#include "base/Log.h"
#include "net/EventLoop.h"
#include "net/SocketOps.h"

namespace ham
{
    namespace net
    {
        Connector::Connector(EventLoop* loop, const InetAddress& serverAddr) 
            : loop_(loop),
              serverAddr_(serverAddr),
              state_(kDisconnected),
              connect_(false),
              retryDelayMs_(kInitRetryDelayMs),
              retryTimerId_(nullptr, -1),
              validTimer_(false)
        {
            INFO("Connector {} ctor", fmt::ptr(this));
        }
        
        Connector::~Connector() 
        {
            INFO("Connector {} dtor", fmt::ptr(this));
            if (connectChannel_)  // channel不为空，说明还没返回可写就进入析构
            {
                stop();     // 那么说明现在在connecting，需要先停止
            }
            assert(!connectChannel_);
        }
        
        void Connector::start() 
        {
            connect_ = true;
            loop_->runInLoop(std::bind(&Connector::startInLoop, this));
        }
        
        void Connector::restart() 
        {
            loop_->assertInLoopThread();
            state_ = kDisconnected;
            connect_ = true;
            retryDelayMs_ = kInitRetryDelayMs;
            startInLoop();
        }
        
        void Connector::stop() 
        {
            connect_ = false;
            loop_->runInLoop(std::bind(&Connector::stopInLoop, this));
            if(validTimer_)
            {
                loop_->cancelTimer(retryTimerId_);  // 取消定时器，避免之后再触发
            }
            
        }
        
        void Connector::startInLoop() 
        {
            loop_->assertInLoopThread();
            assert(state_ == kDisconnected);

            if(connect_)
            {
                connect();
            }
            else
            {
                DEBUG("Do not connect.");
            }
        }
        
        void Connector::stopInLoop() 
        {
            loop_->assertInLoopThread();
            // 如果是已经建立连接，那么channel已经为空，fd由TcpConn负责；这里只负责正在连接的情况
            if(state_ == kConnecting)  
            {
                state_ = kDisconnected;
                int fd = removeAndResetChannel();
                sockets::close(fd);
            }
        }
        
        void Connector::connect() 
        {
            int socket_fd = sockets::createNonblockingOrDie();
            int ret = sockets::connect(socket_fd, serverAddr_.getSockAddrIn());
            int savedErrno = (ret == 0) ? 0 : errno;
            switch (savedErrno) // connect会有很多情况，一一分析
            {
                // 此类状况说明连接已建立
                case 0:   // 通路建立成功
                case EISCONN:  // 已经连接到该套接字
                case EINPROGRESS: // 非阻塞，需要通过可写状态来确定连接是否成功
                case EINTR:  // 系统调用的执行由于捕获中断而中止
                    establishConnection(socket_fd);
                    break;

                // 此类状况说明通路有软性错误，尝试重连
                case EAGAIN:  
                case EADDRINUSE:     // 本地地址处于使用状态
                case EADDRNOTAVAIL:  // 没有可用的端口可用（被bind占用、在使用、在TIME_WAIT但没开启reuse）
                case ECONNREFUSED:   // 发送SYN后收到RST，对端没有监听
                case ENETUNREACH:   
                    retry(socket_fd);
                    break;

                // 不能重连，关闭sockfd
                case EACCES:
                case EPERM:
                case EAFNOSUPPORT:
                case EALREADY:
                case EBADF:
                case EFAULT:
                case ENOTSOCK:
                    ERROR("connect error in Connector::startInLoop {}", savedErrno);
                    sockets::close(socket_fd);	
                    break;

                default:
                    ERROR("connect error in Connector::startInLoop {}", savedErrno);
                    sockets::close(socket_fd);	
                    break;
            }
        }
        
        void Connector::establishConnection(int sockfd) 
        {
            state_ = kConnecting;
            assert(!connectChannel_);

            connectChannel_.reset(new Channel(loop_, sockfd));
            // Connector 只负责建立连接，所以只关注连接过程中可写和错误事件
            connectChannel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
            connectChannel_->setErrorCallback(std::bind(&Connector::handleError, this));
            connectChannel_->enableWriting();  // 监听可写事件，判断连接是否成功
        }
        
        void Connector::retry(int sockfd) 
        {
            assert(state_ == kConnecting);
            sockets::close(sockfd);
            state_ = kDisconnected;
            if(connect_)
            {
                // 避免Connector在等待过程中被析构，使用shared_ptr
                // TODO:既然已经在stop()中取消了定时器，是否可以直接用this？
                retryTimerId_ = loop_->runAfter(retryDelayMs_/1000.0, 
                                                std::bind(&Connector::startInLoop, shared_from_this()));
                validTimer_ = true;
                if(retryDelayMs_ == kMaxRetryDelayMs)
                {
                    WARN("Retry Delay now is max({})", kMaxRetryDelayMs/1000);
                }
                retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
            }
            else
            {
                DEBUG("Do not connect.");
            }
        }
        
        void Connector::handleWrite() 
        {
            if(state_ == kConnecting)
            {
                int socket_fd = removeAndResetChannel(); // 从epoller中移除关注，并将channel置空；否则busy loop
                // socket可写并不意味着连接一定建立成功
                // 还需要用getsockopt(sockfd, SOL_SOCKET, SO_ERROR, ...)再次确认一下。
                int err = sockets::getSocketError(socket_fd);
                if(err)  // 有错误
                {
                    WARN("Connector::handleWrite - SO_ERROR = {}", strerror_tl(err));
                    retry(socket_fd); 
                }
                else if(sockets::isSelfConnect(socket_fd))  // 无错误，自连接
                {
                    WARN("Connector::handleWrite - Self-Connecting occurs");
                    retry(socket_fd);
                }
                else // 无错误
                {
                    state_ = kConnected;
                    if(connect_)
                    {
                        newConnCb_(socket_fd);
                    }
                    else
                    {
                        sockets::close(socket_fd);
                    }
                }
            }
            else
            {
                assert(state_ == kDisconnected);
            }
        }
        
        void Connector::handleError() 
        {
            assert(state_ == kConnecting);
            int socket_fd = removeAndResetChannel();
            int err = sockets::getSocketError(socket_fd);
            
            WARN("Connector::handleWrite - SO_ERROR = {}", strerror_tl(err));
            retry(socket_fd); 
        }
        
        int Connector::removeAndResetChannel() 
        {
            int fd = connectChannel_->getFd();
            connectChannel_->disableAll();
            connectChannel_->remove();
            // Can't reset channel_ here, because we are inside Channel::handleEvent
            loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
            return fd;
        }
        
        void Connector::resetChannel() 
        {
            connectChannel_.reset();
        }
    }
}