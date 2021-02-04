#include "net/TcpServer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"
#include "base/Log.h"
#include <iostream>

using namespace ham;
using namespace ham::net;

void connectionCallback(const TcpConnectionPtr& conn)
{
    if(conn->isConnected())
    {
        std::cout << "New conn comes from " << conn->peerAddress().getIpPortStr()
             << " and its name is " << conn->getName() << std::endl;
    }
    else
    {
        std::cout << "Conn " << conn->getName() << " is down..." << std::endl;
    }
}

void msgCallback(const TcpConnectionPtr& conn, 
                const char* data, ssize_t len)
{
    std::cout << "New message from " << conn->getName()
             << " and its len is " <<  len << std::endl;
}                                

int main()
{
    if(!Logger::Instance().init("Logger", "./log.txt", Logger::logLevel::trace))
    {
        return -1;
    }

    EventLoop loop;
    InetAddress listenAddr(8888);
    TcpServer server(&loop, listenAddr, "TcpServer");

    server.setConnectionCallback(connectionCallback);
    server.setMessageCallback(msgCallback);

    server.start();

    loop.loop();
    return 0;
}