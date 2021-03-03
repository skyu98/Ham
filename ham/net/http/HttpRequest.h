#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__
#include "base/copyable.h"
#include "base/Timestamp.h"
#include <map>
#include <string>

namespace ham
{
namespace net
{

class HttpRequest
{
public:
    enum Method
    {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };

    enum Version
    {
        kUnknown, kHttp10, kHttp11
    };

public:
    HttpRequest()
        : method_(kInvalid),
          version_(kUnknown)
    {}
    

private:
    /* data */
    Method method_;
    Version version_;
    Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;  // 按名字：内容存放
};

HttpRequest::HttpRequest(/* args */)
{
}

HttpRequest::~HttpRequest()
{
}

}
}

#endif // __HTTPREQUEST_H__