#include "base/Log.h"

using namespace ham;

int main()
{
    if(!Logger::Instance().init("Logger", "log/log.txt", spdlog::level::warn))
    {
        // printf("111\n");
        return -1;
    }
    
    Logger::Instance().setLevel(spdlog::level::trace);
    TRACE("TRACE");
    DEBUG("DEBUG");
    INFO("INFO");
    WARN("WARN");
    DEBUG("3 {} ", 1);
    CRITICAL("CRITICAL");

    return 0;
}