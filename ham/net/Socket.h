#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <boost/noncopyable.hpp>
#include <netinet/tcp.h>
namespace ham
{
namespace net
{

class InetAddress;

class Socket : public boost::noncopyable
{
public:
    explicit Socket(int fd);
    Socket(Socket&& src);
    ~Socket();

    void bindAddress(const InetAddress& addr);
    void listen();
    int accept(InetAddress& peerAddr);
    void shutdownWrite();

    int getFd() const { return sockfd_; }

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);

private:
    const int sockfd_;
};
}   
}

#endif // __SOCKET_H__