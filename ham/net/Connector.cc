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
              retryDelayMs_(kInitRetryDelayMs)
        {
            INFO("Connector {} ctor", fmt::ptr(this));
        }
        
        Connector::~Connector() 
        {
            
        }
        
        void Connector::start() 
        {
            connect_ = true;
            loop_->runInLoop(std::bind(&Connector::startInLoop, this));
        }
        
        void Connector::restart() 
        {
            
        }
        
        void Connector::stop() 
        {
            
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
            
        }
        
        void Connector::connect() 
        {
            int socket_fd = sockets::createNonblockingOrDie();
            int ret = sockets::connect(socket_fd, serverAddr_.getSockAddrIn());
            int savedErrno = (ret == 0) ? 0 : errno;
            switch (savedErrno) // connect会有很多情况，一一分析
            {
                case 0:   // 通路建立成功
                case EISCONN:
                case EINPROGRESS:
                case EINTR:  // 系统调用的执行由于捕获中断而中止
                /* code */
                    break;
            
                default:
                    break;
            }
        }
        
        void Connector::tryEstablishConnection() 
        {
            
        }
    }
}