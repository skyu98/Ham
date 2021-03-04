#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__
#include "base/copyable.h"
#include "base/Timestamp.h"
#include <map>
#include <string>
#include <assert.h>

namespace ham
{
namespace net
{

class HttpRequest : public copyable
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
    
    Method method() const { return method_; }
    const char* methodToString() const;
    bool setMethod(const char* start, const char* end);

    Version version() const { return version_; }
    void setVersion(Version newVersion) { version_ = newVersion; }

    Timestamp receiveTime() const { return receiveTime_; }
    void setReceiveTime(Timestamp receiveTime) { receiveTime_ = receiveTime; }

    const std::string& path() const { return path_; }
    void setPath(const char* start, const char* end) { path_.assign(start, end); }

    const std::string& query() const { return query_; }
    void setQuery(const char* start, const char* end) { query_.assign(start, end); }

    const std::string& body() const { return body_; }
    void setBody(const std::string& body) { body_ = body; }
    void setBody(std::string&& body) { body_ = std::move(body); }

    const std::map<std::string, std::string>& headers() const { return headers_; }
    void addHeader(const char* start, const char* colon, const char* end);
    std::string getHeader(const std::string& field) const;

    void swap(HttpRequest& that)
  {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    // query_.swap(that.query_);
    receiveTime_.swap(that.receiveTime_);
    headers_.swap(that.headers_);
  }
    
private:
    /* data */
    Method method_;
    Version version_;
    Timestamp receiveTime_;
    std::string path_;
    std::string query_; // GET请求时所带的参数；在?之后，Key-Value形式
    std::string body_;
    std::map<std::string, std::string> headers_;  // 按名字：内容存放
};

}
}

#endif // __HTTPREQUEST_H__