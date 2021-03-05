#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "base/Log.h"
namespace ham
{
    namespace net
    {
        HttpServer::HttpServer(EventLoop* loop, 
                        const InetAddress& listenAddr,
                        const std::string name) 
            : loop_(loop),
              server_(loop, listenAddr, name)
        {
            server_.setConnectionCallback(
                std::bind(&HttpServer::onConnection, this,
                std::placeholders::_1));
            server_.setMessageCallback(
                std::bind(&HttpServer::onMessage, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3));
        }
        
        void HttpServer::start() 
        {
            INFO("HttpServer[{}] is running at {}", 
                                server_.getName(),
                                server_.getHostPost());
            server_.start();
        }
        
        void HttpServer::onConnection(const TcpConnectionPtr& conn) 
        {
            if (conn->isConnected())
            {
                conn->setContext(HttpContext());
            }
        }
        
        void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer& inputBuffer, Timestamp receiveTime) 
        {
            HttpContext context = boost::any_cast<HttpContext>(conn->getMutableContext());
            if(!context.parseRequest(inputBuffer, receiveTime))
            {
                conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
                conn->shutdown();
            }

            if(context.gotAll())
            {
                onRequest(conn, context.request());
                context.reset();  // 为了长连接，短链接直接就断了，不用重置
            }
            
        }
        
        void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& request) 
        {
            auto connType = request.getHeader("Connection");
            bool isShortConn = (connType == "close") ||
                                (request.version() == HttpRequest::kHttp10 
                                && connType != "Keep-Alive");

            HttpResponse response(isShortConn);
            httpCallback_(request, response);

            Buffer buf;
            response.appendToBuffer(buf);
            conn->send(buf);
            
            if(response.closeConnection())
            {
                conn->shutdown();
            }
        }
    }
}