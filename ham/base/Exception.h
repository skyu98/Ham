#ifndef HAM_BASE_EXCEPTION_H
#define HAM_BASE_EXCEPTION_H

#include "../base/Types.h"
#include <exception>

namespace ham
{

class Exception : public std::exception
{
 public:
  explicit Exception(const char* what);
  explicit Exception(const string& what);
  virtual ~Exception() noexcept;
  virtual const char* what() const noexcept;
  const char* stackTrace() const noexcept;

 private:
  void fillStackTrace();
  string demangle(const char* symbol);

  string message_;      // 错误信息
  string stack_;        // 保存函数名称
}; 

}

#endif  // MUDUO_BASE_EXCEPTION_H
