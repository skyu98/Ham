#include "net/EventLoop.h"
#include "net/TcpClient.h"
#include "net/Channel.h"
#include "base/Util.h"
#include <functional>
#include <string>
#include <memory>
#include <iostream>

using namespace ham;
using namespace ham::net;

class TestClient
{
public:
    TestClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const std::string& name = "TestClient")
        : loop_(loop),
          client_(loop, serverAddr, name),
          STDIN_channel_(util::make_unique<Channel>(loop, STDIN_FILENO))
        {
            client_.setConnectionCallback(std::bind(&TestClient::onConnection, this,
                                        std::placeholders::_1));
            client_.setMessageCallback(std::bind(&TestClient::onMessage, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));
            STDIN_channel_->setReadCallback(std::bind(&TestClient::handleRead, this));
            STDIN_channel_->enableReading();
        }
    
    void connect()
    {
        client_.connect();
    }
    
    void onMessage(const TcpConnectionPtr& conn, Buffer& buf, Timestamp receiveTime)
    {
        std::string msg(buf.retrieveAllAsString());
        std::cout << "New message from " << conn->getName() << std::endl
                << "it says:" << msg
                << " and its len is " << msg.size() << std::endl;
    }

    void onConnection(const TcpConnectionPtr& conn)
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

    void handleRead()
    {
        std::string stdinMsg;
        std::getline(std::cin, stdinMsg);
        client_.getConnection()->send(std::move(stdinMsg));
    }
    
private:
    EventLoop* loop_;
    TcpClient client_;
    std::unique_ptr<Channel> STDIN_channel_;
};

int main()
{
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 8888);
    TestClient client(&loop, serverAddr);
    client.connect();
    loop.loop();
}