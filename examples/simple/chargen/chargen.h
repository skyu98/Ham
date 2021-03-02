#ifndef __CHARGEN_H__
#define __CHARGEN_H__

#include "ham/net/TcpServer.h"
#include "ham/net/Callbacks.h"
#include <string>

using namespace ham;
using namespace ham::net;

class ChargenServer
{
public:
    ChargenServer(EventLoop* loop,
                const InetAddress& listenAddr,
                bool printOut = false);
    void start()
    {
        server_.start();
    }
private:
    void onMessage(const TcpConnectionPtr&conn,
                Buffer& buf,
                Timestamp receiveTime);

    void onConnection(const TcpConnectionPtr&conn);
    void onWriteComplete(const TcpConnectionPtr& conn);
    void printThroughput();

private:
    TcpServer server_;
    std::string message_;
    Timestamp startTime_;
    int64_t transferred_;
};

#endif // __CHARGEN_H__