#include "net/Socket.h"
#include "net/SocketOps.h"
#include "net/InetAddress.h"

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
            
        }
        
        int Socket::accept() 
        {
            
        }
        
        void Socket::shutdownWrite() 
        {
            
        }
    }
}