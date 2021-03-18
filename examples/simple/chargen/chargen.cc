#include "chargen.h"
#include "ham/net/EventLoop.h"
#include <functional> 
using namespace ham;
using namespace ham::net;

ChargenServer::ChargenServer(EventLoop* loop,
                const InetAddress& listenAddr,
                bool printOut) 
    : server_(loop, listenAddr, "ChargenServer"),
      startTime_(Timestamp::now()),
      transferred_(0)
{
    server_.setConnectionCallback(
        std::bind(&ChargenServer::onConnection, this,
                std::placeholders::_1));
    server_.setWriteCompleteCallback(
        std::bind(&ChargenServer::onWriteComplete, this,
                std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&ChargenServer::onMessage, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3));
    
    if (printOut) 
    {
        loop->runEvery(3.0, std::bind(&ChargenServer::printThroughput, this));
    }

    std::string line;
    for (int i = 33; i < 127; ++i)
    {
        line.push_back(char(i));
    }
    line += line;
    for (size_t i = 0; i < 127-33; ++i)
    {
        message_ += line.substr(i, 72) + '\n';
    }
}


void ChargenServer::onMessage(const TcpConnectionPtr&conn,
                Buffer& buf,
                Timestamp receiveTime) 
{
    std::string msg(buf.retrieveAllAsString());
    INFO("{} discards {} bytes received at {}",
            conn->getName(), msg.size(), receiveTime.toFormatTime());
}

void ChargenServer::onConnection(const TcpConnectionPtr&conn) 
{
    INFO("ChargenServer - {} -> {} is {}", 
        conn->peerAddress().getIpPortStr(), 
        conn->localAddress().getIpPortStr(),
        (conn->isConnected() ? "UP" : "DOWN"));
    if(conn->isConnected()) {
        conn->setTcpNoDelay(true);
        conn->send(message_);
    }
}

void ChargenServer::onWriteComplete(const TcpConnectionPtr& conn) 
{
    // INFO("writeCompelete callback()");
    transferred_ += message_.size();
    conn->send(message_);
}

void ChargenServer::printThroughput() 
{
    Timestamp endTime(Timestamp::now());
    double time = timeDifference(endTime, startTime_);
    std::cout << static_cast<double>(transferred_)/time/1024/1024 << " MiB/s" << std::endl;
    transferred_ = 0;
    startTime_ = endTime;
}
