#include "SocketOps.h"
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
int sockets::createNonblockingOrDie(){
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

void sockets::bindOrDie(int sockfd, const sockets::SA_in& address){
	int ret = bind(sockfd, sockaddr_cast(&address),
                static_cast<socklen_t>(sizeof(address)));
	if(ret < 0)
	{
		ERROR("sockets::bindOrDie error");
	}
};

void sockets::listenOrDie(int socket)
{
	int ret = ::listen(socket, SOMAXCONN);
	if(ret < 0)
	{
		ERROR("sockets::listenOrDie error");
	}
}

ssize_t sockets::read(int fd, void *buf, size_t nbyte){
	ssize_t ret = 0;

again:
	ret = ::read(fd, buf, nbyte);
	if(ret == -1)
	{
		if(errno == EINTR)
		{
			goto again;
		}
		else
		{
			ERROR("read error");
		}
	}
	return ret;
}

ssize_t sockets::write(int fd, void *buf, size_t nbyte){
	ssize_t ret = 0;

again:
	ret = ::write(fd, buf, nbyte);
	if(ret == -1)
	{
		if(errno == EINTR)
		{
			goto again;
		}
		else
		{
			ERROR("write error");
		}
	}
	return ret;
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

std::string sockets::toIpStr(const SA_in* addr) {
    char buf[32];
    ::inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(sizeof(buf)));
    return buf;
}

void sockets::close(int sockfd) {
    if(::close(sockfd) < 0) {
        ERROR("sockets::close");
    }
}

void sockets::shutdownWrite(int sockfd) {
    if(::shutdown(sockfd, SHUT_WR) < 0) {
        ERROR("sockets::shutdownWrite");
    }
}

SA_in sockets::getLocalAddr(int sockfd) {
    SA_in localaddr;
    bzero(&localaddr, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if(::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        ERROR("sockets::getLocalAddr");
    }
    return localaddr;
}



}
}
}


