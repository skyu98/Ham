#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__
#include "base/copyable.h"
#include <string>
#include <map>

namespace ham
{
namespace net
{
class Buffer;
class HttpResponse : public copyable
{
public:
    enum Version
    {
        kUnknown, kHttp10, kHttp11
    };

    enum StateCode
    {
        kUnknown,
        k200_Ok = 200,
        k301_MovedPermanently = 301,
        k400_BadRequest = 400,
        k404_NotFound = 404
    };

    explicit HttpResponse(bool closeConn)
        : version_(kUnknown),
          stateCode_(StateCode::kUnknown),
          closeConnection_(closeConn)
          {}

    void setStateCode(StateCode code){ stateCode_ = code; }
    void setStateMessage(const std::string& message){ stateMessage_ = message; }
    void setBody(const std::string& body){ body_ = body; }
    void setBody(std::string&& body){ body_ = std::move(body); }
    void setCloseConnection(bool on){ closeConnection_ = on; }
    void setContentType(const std::string& contentType)
    {
        addHeader("Content-Type", contentType);
    }

    void addHeader(const std::string& field, const std::string& value)
    {
        headers_[field] = value;
    }

    bool closeConnection() const { return closeConnection_; }

    void appendToBuffer(Buffer& outputBuffer) const;

private:
    Version version_;   // 版本号
    StateCode stateCode_;  // 状态码
    std::string stateMessage_;  // 状态信息
    std::string body_;
    bool closeConnection_;
    std::map<std::string, std::string> headers_; 
};
}
}
#endif // __HTTPRESPONSE_H__