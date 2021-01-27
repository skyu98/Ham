#include "base/Log.h"
namespace ham
{
    Logger& Logger::Instance() 
    {
        static Logger logger_;
        return logger_;
    }
    
    bool Logger::init(const std::string& loggerName,
                const std::string& filePath,
                logLevel level) 
    {
        if(isInitialized_)
            return true;
        try
        {
            /*  每一个logger中包含一个存有一个或多个std::shared_ptr<spdlog::sink>的 vector
                1.创建console_sink、rotating_file_sink
                2.创建logger
                3.set_level()等
                4.set_default_logger()覆盖默认注册的全局logger
            */
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            // rotating_file_sink(filename_t base_filename, std::size_t max_size, std::size_t max_files, bool rotate_on_open = false);
            // 达到最大文件大小时，关闭文件，重命名文件并创建新文件。 最大文件大小和最大文件数都可以在构造函数中配置。
            auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, 1024*1024*5, 5, false);
            
            std::vector<spdlog::sink_ptr> sinks{console_sink, rotating_file_sink};

            auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());

            logger->set_level(static_cast<spdlog::level::level_enum>(level));
            logger->set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");
            logger->flush_on(spdlog::level::warn);

            spdlog::set_default_logger(logger);

            //printf("the level is %d\n", spdlog::get(loggerName)->level());
        }
        catch(const spdlog::spdlog_ex& ex)
        {
            std::cerr << ex.what() << std::endl;
            return false;
        }
        isInitialized_ = true;
        return true;
    }
    
    void Logger::setLevel(const logLevel& level) 
    {
        spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
    }
}