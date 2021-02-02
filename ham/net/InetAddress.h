#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__
#include "base/copyable.h"
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstddef>
#include <cstring>
#include "net/Endian.h"
#include "net/SocketOps.h"
#include <string>

namespace ham
{
namespace net
{

class InetAddress : public ham::copyable
{
    typedef struct sockaddr_in SA_in;
public:
    explicit InetAddress(uint16_t port = 0);
    InetAddress(const std::string& ip, uint16_t port);
    explicit InetAddress(const SA_in& addr);

    const SA_in& getSockAddrIn() const { return addr_in_; }
    void setSockAddrIn(const SA_in& addr_in) { addr_in_ = addr_in; }

    uint32_t getIpNetEndian() const { return addr_in_.sin_addr.s_addr; }
    uint16_t getPortNetEndian() const { return addr_in_.sin_port; }
    sa_family_t getFamily() const { return addr_in_.sin_family; }
    
    uint16_t getPort() const { return sockets::networkToHost16(getPortNetEndian()); }
    std::string getIpStr() const;
    std::string getPortStr() const;
    std::string getIpPortStr() const;
    

private:
    SA_in  addr_in_;
};
}
}
#endif // __INETADDRESS_H__