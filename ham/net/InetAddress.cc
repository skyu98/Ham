#include "net/InetAddress.h"

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
#pragma GCC diagnostic error "-Wold-style-cast"

namespace ham
{
    namespace net
    {
        // https://www.cnblogs.com/xiongxinxzy/p/13786047.html
        static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in), 
                        "InetAddress is same size as sockaddr_in");
        static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
        static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");

        InetAddress::InetAddress(uint16_t port) 
        {
            static_assert(offsetof(InetAddress, addr_in_) == 0, "addr_ offset 0");
            bzero(&addr_in_, sizeof(addr_in_));
            addr_in_.sin_family = AF_INET;
            addr_in_.sin_addr.s_addr = sockets::hostToNetwork32(kInaddrAny);
            addr_in_.sin_port = sockets::hostToNetwork16(port);
        }
        
        InetAddress::InetAddress(const std::string& ip, uint16_t port) 
        {
            bzero(&addr_in_, sizeof(addr_in_));
            sockets::ipPortToAddrin(ip, port, &addr_in_);
        }
        
        InetAddress::InetAddress(const SA_in& addr)
            : addr_in_(addr) 
        {}
    
        std::string InetAddress::getIpStr() const
        {
            return sockets::toIpStr(&addr_in_);
        }
        
        std::string InetAddress::getPortStr() const
        {
            return std::to_string(getPort());
        }
        
        std::string InetAddress::getIpPortStr() const
        {
            return getIpStr() + ":" + getPortStr();
        }
    }
}