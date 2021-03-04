#ifndef __HTTPCONTEXT_H__
#define __HTTPCONTEXT_H__
#include "HttpRequest.h"
#include "base/Timestamp.h"
#include "base/copyable.h"
namespace ham
{
namespace net
{
class Buffer;
class HttpContext :public copyable
{
public:
    enum ParseState
    {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll
    };

    HttpContext(): parseState_(kExpectRequestLine){}

    bool parseHttpRequest(Buffer& inputBuffer, Timestamp receiveTime);

    const HttpRequest& request() const { return request_; }
    HttpRequest& request() { return request_; }
    bool gotAll() const { return parseState_ == kGotAll; }

    HttpContext& reset()
    {
        HttpRequest dummy;
        request_.swap(dummy);
        return *this;
    }
private:
    bool parseRequestLine(const char* begin, const char* end);

private:
    /* data */
    HttpRequest request_;
    ParseState parseState_;
};


}
}
#endif // __HTTPCONTEXT_H__