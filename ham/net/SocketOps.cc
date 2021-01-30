#include "SocketOps.h"
#include "base/Log.h"
using namespace ham::net;


// Socket functions
int socket::socket(){
	int sockfd = 0;
	// sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        ERROR("sockets::createNonblockingOrDie");
    }
	return sockfd;
}

int socket::bind(int sockfd, const struct sockaddr_in& address){
	int ret = 0;
	//ret = bind(sockfd, static_cast<SA*>((void*)&address), sizeof(address));
	if(ret == -1)
	{
		ERROR("bind error");
	}
	return ret;
};

int socket::listen(int socket, int backlog){
	int ret = 0;
	ret = listen(socket, backlog);
	if(ret == -1)
	{
		ERROR("listen error");
	}
	return ret;
}

ssize_t socket::read(int fd, void *buf, size_t nbyte){
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

ssize_t socket::write(int fd, void *buf, size_t nbyte){
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


