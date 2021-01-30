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

namespace ham
{
namespace net
{
namespace socket
{

typedef struct sockaddr SA;

//void print_err(const char* str);

int socket();

int bind(int sockfd, const struct sockaddr_in& address);

int listen(int socket, int backlog = 128);

ssize_t read(int fd, void *buf, size_t nbyte);
ssize_t write(int fd, void *buf, size_t nbyte);

}
}
}



#endif // __SOCKETOPS_H__