#ifndef __SOCKETOPS_H__
#define __SOCKETOPS_H__
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

namespace ham
{
namespace net
{
namespace sockets
{

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_in;

const SA* sockaddr_cast(const SA_in* addr);
SA* sockaddr_cast(SA_in* addr);
const SA_in* sockaddr_in_cast(const SA* addr);
SA_in* sockaddr_in_cast(SA* addr);

int createNonblockingOrDie();
void bindOrDie(int sockfd, const SA_in& addrin);
void listenOrDie(int socket);
int  accept(int sockfd, SA_in& addrin);

ssize_t read(int fd, void *buf, size_t nbyte);
ssize_t write(int fd, void *buf, size_t nbyte);
ssize_t write(int fd, std::string msg);

void close(int sockfd);
void shutdownWrite(int sockfd);
void ipPortToAddrin(const std::string& ip, uint16_t port, struct sockaddr_in* addr);
std::string toIpStr(const struct sockaddr_in* addr);
struct sockaddr_in getLocalAddr(int sockfd);

}
}
}



#endif // __SOCKETOPS_H__