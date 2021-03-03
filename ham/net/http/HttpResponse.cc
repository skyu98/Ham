#include "HttpResponse.h"
#include "net/Buffer.h"
#include <stdio.h>
namespace ham
{
    namespace net
    {
        void HttpResponse::appendToBuffer(Buffer& outputBuffer) const 
        {
            char buf[32];
            // response line
            outputBuffer.append("Http/1.");
            if(version_ == kHttp10)
            {
                outputBuffer.append("0 ");
            }
            else
            {
                outputBuffer.append("1 ");
            }
            outputBuffer.append(std::to_string(static_cast<int>(stateCode_)));
            outputBuffer.append(stateMessage_);
            outputBuffer.append("\r\n");

            if(closeConnection_)  // 如果是短连接，则不存在粘包问题，即不需要发送长度
            {
                outputBuffer.append("Connection: close\r\n");
            }
            else
            {
                // 否则header需要包含Content-Length
                snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
                outputBuffer.append(buf);
                outputBuffer.append("Connection: Keep-Alive\r\n");
            }

            // headers
            for(const auto& header : headers_)
            {
                outputBuffer.append(header.first);
                outputBuffer.append(": ");
                outputBuffer.append(header.second);
                outputBuffer.append("\r\n");
            }

            outputBuffer.append("\r\n");

            // body
            outputBuffer.append(body_);
        }
    }
}