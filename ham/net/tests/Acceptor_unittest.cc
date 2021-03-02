#include "net/EventLoop.h"
#include "net/Acceptor.h"
#include "net/InetAddress.h"

using namespace ham;
using namespace ham::net;

void newConn(int fd, const InetAddress& peerAddr)
{
    std::cout << "newConnection() : accepted a new connection from "
        << peerAddr.getIpPortStr() << std::endl;
    sockets::write(fd, "How are you?\n");
    sockets::close(fd);
}

int main()
{
    std::cout << "main() : pid = " << ::getpid() << std::endl;
    InetAddress listenAddr(8888);
    EventLoop loop;
    
    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnCallback(newConn);
    acceptor.listen();

    loop.loop();
    return 0;
}