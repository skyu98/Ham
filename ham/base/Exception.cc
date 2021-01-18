// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "../base/Exception.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>

using namespace ham;

Exception::Exception(const char* msg)
  : message_(msg)
{
  fillStackTrace();
}

Exception::Exception(const string& msg)
  : message_(msg)
{
  fillStackTrace();
}

Exception::~Exception() noexcept
{
}

const char* Exception::what() const noexcept     // 错误信息
{
  return message_.c_str();
}

const char* Exception::stackTrace() const noexcept   // 函数栈中的函数名称
{
  return stack_.c_str();
}

void Exception::fillStackTrace()     // 生成一个函数栈，里面保存运行到exception处的经过的函数，保存到stack_当中
{
  const int len = 200;
  void* buffer[len];
  int nptrs = ::backtrace(buffer, len);  // 返回获取到的调用栈个数
  char** strings = ::backtrace_symbols(buffer, nptrs);   // 这个指针数组是由malloc生成的，需要我们手动释放
  if (strings)
  {
    for (int i = 0; i < nptrs; ++i)
    {
	    stack_.append(demangle(strings[i]));   // 运行的结果是解码失败，暂时使用未解码的名称
      stack_.push_back('\n');
    }
    free(strings);   // 在此处释放malloc的内存
  }
}

string Exception::demangle(const char* symbol)
{
  size_t size;
  int status;
  char temp[128];
  char* demangled;
  //first, try to demangle a c++ name
  if (1 == sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp)) {
    if (NULL != (demangled = abi::__cxa_demangle(temp, NULL, &size, &status))) {
      string result(demangled);
      free(demangled);
      return result;
    }
  }
  //if that didn't work, try to get a regular c symbol
  if (1 == sscanf(symbol, "%127s", temp)) {
    return temp;
  }
 
  //if all else fails, just return the symbol
  return symbol;
}
