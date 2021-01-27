#include "base/Log.h"

using namespace ham;

int main()
{
    // 文件路径是相对于<运行>当前程序的位置而言
    if(!Logger::Instance().init("Logger", "./log.txt", Logger::logLevel::trace))
    {
        // printf("111\n");
        return -1;
    }
    
    TRACE("TRACE");
    DEBUG("DEBUG");
    INFO("INFO");
    WARN("WARN");
    DEBUG("3 {} ", 1);
    CRITICAL("CRITICAL");

    Logger::Instance().setLevel(Logger::logLevel::info);

    TRACE("TRACE");
    DEBUG("DEBUG");
    INFO("INFO");
    WARN("WARN");
    DEBUG("3 {} ", 222);
    CRITICAL("CRITICAL");

    return 0;
}