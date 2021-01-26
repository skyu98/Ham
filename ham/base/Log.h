#ifndef __LOG_H__
#define __LOG_H__
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <boost/noncopyable.hpp>
#include <string>
#include <memory>
#include <atomic>
#include <vector>
#include <iostream>

namespace ham
{
class Logger final : public boost::noncopyable
{
public:
    typedef spdlog::level::level_enum  logLevel;
    static Logger& Instance();
    bool init(const std::string& loggerName = "Logger",
            const std::string& filePath = "./log/log.txt",
            logLevel level = spdlog::level::debug);

    void shutdown() { spdlog::shutdown(); }
    void setLevel(const logLevel&);

private:
    std::atomic_bool isInitialized_{false};
};

#define TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__);
#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);
#define INFO(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__);
#define WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);
#define CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__);
}
#endif // __LOG_H__