#include "net/SocketOps.h"
#include "net/Endian.h"
#include "base/Log.h"
#include "base/Types.h"

namespace ham
{
namespace net
{
namespace sockets
{

#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd) {
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;    
    int res = ::fcntl(sockfd, F_SETFL, flags);

    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
}

#endif

// Socket functions
int createNonblockingOrDie(){
#if VALGRIND	
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        CRITICAL("sockets::createNonblockingOrDie");
    }
    setNonBlockAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        CRITICAL("sockets::createNonblockingOrDie");
    }
#endif
	return sockfd;
}

void bindOrDie(int sockfd, const sockets::SA_in& address){
	int ret = bind(sockfd, sockaddr_cast(&address),
                static_cast<socklen_t>(sizeof(address)));
	if(ret < 0)
	{
		ERROR("sockets::bindOrDie error");
	}
};

void listenOrDie(int fd)
{
	int ret = ::listen(fd, SOMAXCONN);
	if(ret < 0)
	{
		ERROR("sockets::listenOrDie error");
	}
}

int accept(int sockfd, SA_in& addrin) 
{
    socklen_t addr_len = static_cast<socklen_t>(sizeof(addrin));
#if VALGRIND || defined (NO_ACCEPT4)
    int connFd = ::accept(sockfd, sockaddr_cast(&addrin), &addr_len);
    setNonBlockAndCloseOnExec(connFd);
#else
    int connFd = ::accept4(sockfd, sockaddr_cast(&addrin),
                            &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if(connFd < 0)
    {
        int savedErrno = errno;
        ERROR("socket::accept");
        switch (savedErrno) 
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                {
                    errno = savedErrno;
                    break;
                }
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                {
                    CRITICAL("unexpected error of ::accept {}", savedErrno);
                    break;
                }
            default:
                {
                    CRITICAL("unknown error of ::accept {}", savedErrno);
                    break;
                }
        }
    }
    return connFd;
}

int connect(int sockfd, const SA_in& serverAddr) 
{
    return ::connect(sockfd, sockaddr_cast(&serverAddr), sizeof(serverAddr)); 
}

ssize_t read(int fd, void *buf, size_t nbyte){
	return ::read(fd, buf, nbyte);
}

// 自连接是指(sourceIP, sourcePort) = (destIP, destPort)
// 自连接发生的原因:
// 客户端在发起connect的时候，没有bind(2)
// 客户端与服务器端在同一台机器，即sourceIP = destIP，
// 服务器尚未开启，即服务器还没有在destPort端口上处于监听
// 就有可能出现自连接，这样，服务器也无法启动了
bool isSelfConnect(int sockfd)
{
  struct sockaddr_in localaddr = getLocalAddr(sockfd);
  struct sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port
      && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

ssize_t readv(int fd, const struct iovec* vec, int iocnt) 
{
    return ::readv(fd, vec, iocnt);
}

ssize_t write(int fd, const void *buf, size_t nbyte){
	return ::write(fd, buf, nbyte);
}

ssize_t write(int fd, const std::string& msg){
    return write(fd, msg.c_str(), msg.length());
}

void close(int sockfd) {
    if(::close(sockfd) < 0) {
        ERROR("sockets::close");
    }
}

void shutdownWrite(int sockfd) {
    if(::shutdown(sockfd, SHUT_WR) < 0) {
        ERROR("sockets::shutdownWrite");
    }
}


const SA* sockaddr_cast(const SA_in* addr) 
{
    return static_cast<const SA*>(implicit_cast<const void*>(addr));
}

SA* sockaddr_cast(SA_in* addr) 
{
    return static_cast<SA*>(implicit_cast<void*>(addr));
}

const SA_in* sockaddr_in_cast(const SA* addr) 
{
    return static_cast<const SA_in*>(implicit_cast<const void*>(addr));
}

SA_in* sockaddr_in_cast(SA* addr) 
{
    return static_cast<SA_in*>(implicit_cast<void*>(addr));
}

std::string toIpStr(const SA_in* addr) {
    char buf[32];
    ::inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(sizeof(buf)));
    return buf;
}

void ipPortToAddrin(const std::string& ip, uint16_t port, SA_in* addr) 
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if(::inet_pton(AF_INET, ip.c_str(), &addr->sin_addr) <= 0) {
        ERROR("sockets::ipPortToAddrin");
    }
}

SA_in getLocalAddr(int sockfd) {
    SA_in localaddr;
    bzero(&localaddr, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if(::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        ERROR("sockets::getLocalAddr");
    }
    return localaddr;
}

SA_in getPeerAddr(int sockfd) 
{
    SA_in peeraddr;
    bzero(&peeraddr, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
    if(::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
        ERROR("sockets::getPeerAddr");
    }
    return peeraddr;
}

int getSocketError(int sockfd) 
{
    int optval;
  socklen_t optlen = sizeof optval;

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    return errno;
  }
  else
  {
    return optval;
  }
}

}
}
}


