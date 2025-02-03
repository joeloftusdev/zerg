// Copyright 2025 Joseph A. Loftus
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef GLOBAL_LOGGER_HPP
#define GLOBAL_LOGGER_HPP

#include "logger.hpp"
#include "constants.hpp"
#include <string>        // std::string
#include <memory>        // std::shared_ptr
#include <mutex>         // std::mutex, std::lock_guard
#include <fstream>       // std::ifstream
#include <sstream>       // std::istringstream
#include <unordered_map> // std::unordered_map

namespace cpp_logger
{

inline std::string &getLogFileName()
{
    static std::string logFileName = "global_logfile.log";
    return logFileName;
}

inline std::string &getLogFilePath()
{
    static std::string logFilePath = "./"; // default to current directory
    return logFilePath;
}

inline void setLogFilePath(const std::string &path) { getLogFilePath() = path; }

inline std::shared_ptr<Logger<DEFAULT_BUFFER_SIZE>> &
getGlobalLogger(const std::string &filename = "")
{
    static std::unordered_map<std::string, std::shared_ptr<Logger<DEFAULT_BUFFER_SIZE>>> instances;
    static std::mutex mtx;

    std::lock_guard<std::mutex> lock(mtx);
    std::string fullPath = getLogFilePath() + (filename.empty() ? getLogFileName() : filename);
    if (instances.find(fullPath) == instances.end())
    {
        instances[fullPath] = std::make_shared<Logger<DEFAULT_BUFFER_SIZE>>(fullPath);
    }
    return instances[fullPath];
}

inline void setGlobalLoggerVerbosity(const Verbosity level)
{
    getGlobalLogger()->setLogLevel(level);
}

inline Verbosity stringToVerbosity(const std::string &level)
{
    static const std::unordered_map<std::string, Verbosity> verbosityMap = {
        {"DEBUG", Verbosity::DEBUG_LVL},
        {"INFO", Verbosity::INFO_LVL},
        {"WARN", Verbosity::WARN_LVL},
        {"ERROR", Verbosity::ERROR_LVL},
        {"FATAL", Verbosity::FATAL_LVL}};
    if (const auto it = verbosityMap.find(level); it != verbosityMap.end())
    {
        return it->second;
    }
    return Verbosity::DEBUG_LVL; // Default verbosity
}

inline void loadConfiguration(const std::string &configFile)
{
    std::ifstream file(configFile);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open configuration file");
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string key;
        if (std::string value; std::getline(iss, key, '=') && std::getline(iss, value))
        {
            if (key == "verbosity")
            {
                setGlobalLoggerVerbosity(stringToVerbosity(value));
            }
            else if (key == "logFilePath")
            {
                setLogFilePath(value);
            }
        }
    }
}

inline void resetGlobalLogger(const std::string &filename = "")
{
    static std::unordered_map<std::string, std::shared_ptr<Logger<DEFAULT_BUFFER_SIZE>>>
        &instances =
            *new std::unordered_map<std::string, std::shared_ptr<Logger<DEFAULT_BUFFER_SIZE>>>;
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    std::string fullPath = getLogFilePath() + (filename.empty() ? getLogFileName() : filename);
    instances.erase(fullPath);
}

// template functions for logging with the global loggers to reduce reliance on macros
template <typename... Args>
constexpr void log(const Verbosity level, const char *file, int line, const std::string &format,
                   Args &&...args)
{
    getGlobalLogger()->log(level, file, line, format.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
constexpr void logWithFile(const Verbosity level, const std::string &loggerFile, const char *file,
                           int line, const std::string &format, Args &&...args)
{
    getGlobalLogger(loggerFile)
        ->log(level, file, line, format.c_str(), std::forward<Args>(args)...);
}

} // namespace cpp_logger

// macros are used here to automatically capture __FILE__ and __LINE__ at the call site
// this ensures that the correct file name and line number are logged
#define cpp_log(level, format, ...)                                                                \
    cpp_logger::log(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define cpp_log_with_file(level, file, format, ...)                                                \
    cpp_logger::logWithFile(level, file, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif // GLOBAL_LOGGER_HPP