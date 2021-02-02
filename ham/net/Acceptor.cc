#include "net/Acceptor.h"
#include "net/InetAddress.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "net/Socket.h"
#include "base/Log.h"
#include "base/Util.h"
namespace ham
{
    namespace net
    {
        Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr, bool reusePort) 
            : loop_(loop),
              listening_(false),
              idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
              acceptSocket_(util::make_unique<Socket>(sockets::createNonblockingOrDie())),
              acceptChannel_(util::make_unique<Channel>(loop, acceptSocket_->getFd()))
        {
            assert(idleFd_ > 0);
            acceptSocket_->setReuseAddr(true);
            acceptSocket_->setReusePort(reusePort);
            acceptSocket_->bindAddress(addr);
            acceptChannel_->setReadCallback(std::bind(&Acceptor::handleRead, this, 
                                            std::placeholders::_1));
        }
        
        Acceptor::~Acceptor() 
        {
            acceptChannel_->disableAll();
            acceptChannel_->remove();
            sockets::close(idleFd_);
        }
        
        void Acceptor::listen() 
        {
            loop_->assertInLoopThread();
            listening_ = true;
            sockets::listenOrDie(acceptSocket_->getFd());
            acceptChannel_->enableReading();
        }
        
        void Acceptor::handleRead(Timestamp time) 
        {
            loop_->assertInLoopThread();
            InetAddress peerAddr;
            int connfd = acceptSocket_->accept(peerAddr);
            if(connfd > 0)
            {
                TRACE("Accept of {}", peerAddr.getIpPortStr());
                if(newConnCallback_)
                {
                    newConnCallback_(connfd, peerAddr);
                }
                else
                {
                    sockets::close(connfd);
                }
            }
            else
            {
                ERROR("in Acceptor::handleRead");
                if(errno == EMFILE)
                {
                    sockets::close(idleFd_);
                    idleFd_ = ::accept(acceptSocket_->getFd(), nullptr, nullptr);
                    sockets::close(idleFd_);
                    idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
                }
            }
        }
    }
}