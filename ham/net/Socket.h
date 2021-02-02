#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <boost/noncopyable.hpp>
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
    int accept();
    void shutdownWrite();


private:
    const int sockfd_;
};
}   
}

#endif // __SOCKET_H__