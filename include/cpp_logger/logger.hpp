#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstdarg>

namespace cpp_logger {

enum class Verbosity {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

template<std::size_t MaxFileSize, Verbosity LogLevel>
class Logger {
public:
    explicit Logger(const std::string& filename)
        : _filename(filename), _current_size(0) {
        rotateLogFile();
    }

    ~Logger() {
        if (_logfile.is_open()) {
            _logfile.close();
        }
    }

    template<typename... Args>
    Logger& log(Verbosity level, const char* file, int line, const char* format, Args... args) {
        if (level >= LogLevel) {
            std::ostringstream oss;
            oss << getCurrentTime() << " [" << getVerbosityString(level) << "] "
                << getFileName(file) << ":" << line << " "
                << formatString(format, args...);
            std::string log_entry = oss.str();
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

    void rotateLogFile() {
        if (_logfile.is_open()) {
            _logfile.close();
        }
        _logfile.open(_filename, std::ios::out | std::ios::app);
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
            case Verbosity::DEBUG: return "DEBUG";
            case Verbosity::INFO: return "INFO";
            case Verbosity::WARN: return "WARN";
            case Verbosity::ERROR: return "ERROR";
            case Verbosity::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    std::string getFileName(const std::string& path) const {
        size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }

    template<typename... Args>
    std::string formatString(const char* format, Args... args) const {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-security"
        int size = std::snprintf(nullptr, 0, format, args...) + 1; 
        if (size <= 0) {
            throw std::runtime_error("Error during formatting.");
        }
        std::vector<char> buf(size);
        std::snprintf(buf.data(), size, format, args...);
    #pragma GCC diagnostic pop
        return std::string(buf.data(), buf.size() - 1); 
    }
};

} // namespace cpp_logger

#endif // LOGGER_HPP