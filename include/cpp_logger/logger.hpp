#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>     // std::ostream
#include <fstream>      // std::ofstream
#include <string>       // std::string
#include <chrono>       // std::chrono::system_clock
#include <ctime>        // std::localtime, std::time_t
#include <iomanip>      // std::put_time
#include <mutex>        // std::mutex, std::lock_guard
#include <algorithm>    // std::remove_if
#include <fmt/core.h>   // fmt::format
#include <fmt/ostream.h>// fmt::runtime


namespace cpp_logger {

enum class Verbosity {
    DEBUG_LVL,
    INFO_LVL,
    WARN_LVL,
    ERROR_LVL,
    FATAL_LVL
};

template<std::size_t MaxFileSize>
class Logger {
public:
    explicit Logger(const std::string& filename, Verbosity logLevel = Verbosity::DEBUG_LVL)
        : _filename(filename), _current_size(0), _log_level(logLevel) {
        openLogFile();
    }

    ~Logger() {
        if (_logfile.is_open()) {
            _logfile.close();
        }
    }

    void setLogLevel(Verbosity level) {
        std::lock_guard<std::mutex> lock(_mutex);
        _log_level = level;
    }

    template<typename... Args>
    Logger& log(Verbosity level, const char* file, int line, const char* format, Args&&... args) {
        std::lock_guard<std::mutex> lock(_mutex); 
        if (level >= _log_level) {
            std::ostringstream oss;
            oss << getCurrentTime() << " [" << getVerbosityString(level) << "] "
                << getFileName(file) << ":" << line << " ";

        // type safe format of the string using fmt::format
        try {
            oss << fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
        } catch (const fmt::format_error& e) {
            oss << "[FORMAT ERROR: " << e.what() << "]";
        }

        std::string log_entry = oss.str();
        sanitizeString(log_entry);
        if (_current_size + log_entry.size() > MaxFileSize) {
            rotateLogFile();
        }
        _logfile << log_entry << std::endl;
        _current_size += log_entry.size();
    }
    return *this;
}

private:
    std::string _filename;
    std::ofstream _logfile;
    std::size_t _current_size;
    Verbosity _log_level;
    std::mutex _mutex;

    void openLogFile() {
        _logfile.open(_filename, std::ios::out | std::ios::app);
        _current_size = _logfile.tellp();
    }

    void rotateLogFile() {
        if (_logfile.is_open()) {
            _logfile.close();
        }
        _logfile.open(_filename, std::ios::out | std::ios::trunc); 
        _current_size = 0;
    }

    std::string getCurrentTime() const {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    std::string getVerbosityString(Verbosity level) const {
        switch (level) {
            case Verbosity::DEBUG_LVL: return "DEBUG";
            case Verbosity::INFO_LVL: return "INFO";
            case Verbosity::WARN_LVL: return "WARN";
            case Verbosity::ERROR_LVL: return "ERROR";
            case Verbosity::FATAL_LVL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    std::string getFileName(const std::string& path) const {
        size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }

    // template<typename... Args> //this is not type safe use fmtlib?
    // std::string formatString(const char* format, Args... args) const {
    // #pragma GCC diagnostic push
    // #pragma GCC diagnostic ignored "-Wformat-security"
    //     int size = std::snprintf(nullptr, 0, format, args...) + 1; 
    //     if (size <= 0) {
    //         throw std::runtime_error("Error during formatting.");
    //     }
    //     std::vector<char> buf(size);
    //     std::snprintf(buf.data(), size, format, args...);
    // #pragma GCC diagnostic pop
    //     return std::string(buf.data(), buf.size() - 1); 
    // }

    void sanitizeString(std::string& str) {
        str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) {
            return !std::isprint(c);
        }), str.end());
    }
};


} // namespace cpp_logger

#endif // LOGGER_HPP