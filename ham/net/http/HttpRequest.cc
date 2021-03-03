#include "HttpRequest.h"
namespace ham
{
    namespace net
    {
        const char* HttpRequest::methodToString() const
        {
            const char* result = "UNKNOWN";
            switch(method_)
            {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
            }
            return result;
        }
    
        bool HttpRequest::setMethod(const char* start, const char* end) 
        {
            assert(method_ == kInvalid);
            string m(start, end);
            if (m == "GET")
            {
                method_ = kGet;
            }
            else if (m == "POST")
            {
                method_ = kPost;
            }
            else if (m == "HEAD")
            {
                method_ = kHead;
            }
            else if (m == "PUT")
            {
                method_ = kPut;
            }
            else if (m == "DELETE")
            {
                method_ = kDelete;
            }
            else
            {
                method_ = kInvalid;
            }
            return method_ != kInvalid;
        }


        void HttpRequest::addHeader(const char* start, const char* colon, const char* end)
        {
            const char* cur = colon;
            std::string field(start, cur);  
            ++cur;
            while(cur < end && isspace(*cur))  //
            {
                ++cur;
            }

            std::string value(cur, end);
            while (!value.empty() && isspace(value[value.size()-1]))
            {
                value.resize(value.size()-1);
            }
            headers_[field] = value;
        }

        std::string HttpRequest::getHeader(const std::string& field) const
        {
            std::string value;
            auto it = headers_.find(field);
            if(it != headers_.end())
            {
                value = it->second;
            }
            return value;
        }
    }
}