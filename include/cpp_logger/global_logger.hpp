#ifndef GLOBAL_LOGGER_HPP
#define GLOBAL_LOGGER_HPP

#include "logger.hpp"
#include <string>
#include <memory>
#include <mutex>

namespace cpp_logger {

inline std::string& getLogFileName() {
    static std::string logFileName = "global_logfile.log";
    return logFileName;
}

inline void setLogFileName(const std::string& filename) {
    getLogFileName() = filename;
}

inline std::shared_ptr<Logger<1024, cpp_logger::Verbosity::DEBUG>>& getGlobalLogger(const std::string& filename = "") {
    static std::shared_ptr<Logger<1024, cpp_logger::Verbosity::DEBUG>> instance;
    static std::mutex mtx;

    std::lock_guard<std::mutex> lock(mtx);
    if (!filename.empty() && getLogFileName() != filename) {
        setLogFileName(filename);
        instance.reset();
        instance = std::make_shared<Logger<1024, cpp_logger::Verbosity::DEBUG>>(getLogFileName());
    } else if (!instance) {
        instance = std::make_shared<Logger<1024, cpp_logger::Verbosity::DEBUG>>(getLogFileName());
    }

    return instance;
}

} // namespace cpp_logger

#define cpp_log(level, format, ...) \
    cpp_logger::getGlobalLogger()->log(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define cpp_log_with_file(level, file, format, ...) \
    cpp_logger::getGlobalLogger(file)->log(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif // GLOBAL_LOGGER_HPP