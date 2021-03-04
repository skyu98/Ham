#include "HttpContext.h"
#include "net/Buffer.h"
#include <string>
#include  <algorithm> 
namespace ham
{
    namespace net
    {
        bool HttpContext::parseHttpRequest(Buffer& inputBuffer, Timestamp receiveTime) 
        {
            bool hasMore = true; 
            bool okWithRequestLine = true;
            
            while(hasMore)  
            {
                if(parseState_ == kExpectRequestLine)
                {
                    auto crlf = inputBuffer.findCRLF();
                    if(!crlf) return false;   // 如果没有crlf，说明格式错误
                    okWithRequestLine = parseRequestLine(inputBuffer.peek(), crlf);
                    if(okWithRequestLine)   // 解析request line成功
                    {
                        request_.setReceiveTime(receiveTime);
                        inputBuffer.retrieveUtil(crlf + 2);  // 将换行符读走
                        parseState_ = kExpectHeaders; // 进入解析header状态
                    }
                    else  // 解析request line失败
                    {
                        hasMore = false;
                    }
                }
                else if(parseState_ == kExpectHeaders)
                {
                    auto crlf = inputBuffer.findCRLF();
                    if(crlf)  // 有换行符，说明至少有一行（header或者空行）
                    {
                        auto colon = std::find(inputBuffer.peek(), crlf, ':');
                        if(colon != crlf)  // 有冒号，说明是header行
                        {
                            request_.addHeader(inputBuffer.peek(), colon, crlf);
                        }
                        else   // 没有冒号，说明是空行
                        {
                            parseState_ = kExpectBody;  // headers解析完毕，解析body
                        }
                        inputBuffer.retrieveUtil(crlf + 2);  // 读走换行符
                    }
                    else   // 没有换行符，说明后面没内容了
                    {
                        hasMore = false;
                        parseState_ = kGotAll;
                    }
                }
                else if(parseState_ == kExpectBody)
                {
                    // 剩下的全是body部分
                    request_.setBody(std::move(inputBuffer.retrieveAllAsString())); 
                    hasMore = false;
                    parseState_ = kGotAll;
                }
            }
            // 如果request line没问题，后面都不会有问题
            // 如果request line有问题，退出循环后直接返回false
            return okWithRequestLine;  
        }
        
        bool HttpContext::parseRequestLine(const char* begin, const char* end) 
        {
            
        }
    }
}