#ifndef FILE_LOGGER_BACKEND_HPP
#define FILE_LOGGER_BACKEND_HPP

//#include "../backend/file_log_backend.hpp"
#include "../logger.hpp"
#include "../constants.hpp"
#include <memory> // std::shared_ptr
#include <mutex>  // std::mutex, std::lock_guard

namespace zerg
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
getFileLogger(const std::string &filename = "")
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
    getFileLogger()->setLogLevel(level);
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

inline void resetFileLogger(const std::string &filename = "")
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
    getFileLogger()->log(level, file, line, format.c_str(), std::forward<Args>(args)...);
}

template <typename... Args>
constexpr void logWithFile(const Verbosity level, const std::string &loggerFile, const char *file,
                           int line, const std::string &format, Args &&...args)
{
    getFileLogger(loggerFile)
        ->log(level, file, line, format.c_str(), std::forward<Args>(args)...);
}



} // namespace zerg

#define cpp_log(level, format, ...)                                                                \
    zerg::log(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define cpp_log_with_file(level, file, format, ...)                                                \
    zerg::logWithFile(level, file, __FILE__, __LINE__, format, ##__VA_ARGS__)



#endif // FILE_LOGGER_BACKEND_HPP