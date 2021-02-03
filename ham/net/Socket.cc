#include "net/Socket.h"
#include "net/SocketOps.h"
#include "net/InetAddress.h"
#include "base/Log.h"

namespace ham
{
    namespace net
    {
        Socket::Socket(int fd) 
            : sockfd_(fd)   
        {
            
        }
        
        Socket::Socket(Socket&& src) 
            : sockfd_(src.sockfd_)
        {
            *(const_cast<int*>(&src.sockfd_)) = -1;
        }
        
        
        Socket::~Socket() 
        {
            sockets::close(sockfd_);
        }
        
        void Socket::bindAddress(const InetAddress& addr) 
        {
            sockets::bindOrDie(sockfd_, addr.getSockAddrIn());
        }
        
        void Socket::listen() 
        {
            sockets::listenOrDie(sockfd_);
        }
        
        int Socket::accept(InetAddress& peerAddr) 
        {   
            struct sockaddr_in addr;
            bzero(&addr, sizeof(addr));
            int connFd = sockets::accept(sockfd_, addr);
            if(connFd > 0)
            {
                peerAddr.setSockAddrIn(addr);
            }
            return connFd;
        }
        
        void Socket::shutdownWrite() 
        {
            sockets::shutdownWrite(sockfd_);
        }

        void Socket::setTcpNoDelay(bool on) {
            int opt = on ? 1 : 0;
            int ret = ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, 
                &opt, static_cast<socklen_t>(sizeof(opt)));
            if(ret < 0 && on) {
                ERROR("Socket::setTcpNoDelay failed.");
            }
        }

        void Socket::setReuseAddr(bool on) {
            int opt = on ? 1 : 0;
            int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                    &opt, static_cast<socklen_t>(sizeof(opt)));
            if(ret < 0 && on) {
                ERROR("Socket::setReuseAddr failed.");
            }
        }

        void Socket::setReusePort(bool on) {
        #ifdef SO_REUSEPORT 
            int opt = on ? 1 : 0;
            int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                        &opt, static_cast<socklen_t>(sizeof(opt)));
            if(ret < 0 && on) {
                ERROR("SO_REUSEPORT failed.");
            }
        #else 
            if(on) {
                ERROR("SO_REUSEPORT is not supported.");
            }
        #endif 
        }
        
        void Socket::setKeepAlive(bool on) 
        {
            int optval = on ? 1 : 0;
            int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                            &optval, static_cast<socklen_t>(sizeof(optval)));
            if(ret < 0 && on) {
                ERROR("Socket::setKeepAlive failed.");
            }
        }
    }
}