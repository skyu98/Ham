#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
#include <exception>
#include <string>
#include <execinfo.h>
#include <cxxabi.h>

namespace ham
{

class Exception: public std::exception
{
public:
    explicit Exception(std::string msg);
    ~Exception() noexcept override {}

    const char* what()  const noexcept override { return msg_.c_str(); } ;
    const char* stackTrace() const noexcept { return stack_.c_str(); };

private:
    std::string fillStack() const noexcept;

private:
    std::string msg_;
    std::string stack_;
};

}

#endif // __EXCEPTION_H__