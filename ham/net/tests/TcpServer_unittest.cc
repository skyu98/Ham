#include "net/TcpServer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"
#include "net/Buffer.h"
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
                Buffer& buf, Timestamp receiveTime)
{
    std::string msg(buf.retrieveAllAsString());
    std::cout << "New message from " << conn->getName() << std::endl
            << "it says:" << msg
            << " and its len is " << msg.size() << std::endl;
    conn->send(msg);
}                                

int main()
{
    EventLoop loop;
    InetAddress listenAddr(8888);
    TcpServer server(&loop, listenAddr, "TcpServer");

    server.setConnectionCallback(connectionCallback);
    server.setMessageCallback(msgCallback);

    server.start();

    loop.loop();
    return 0;
}