#ifndef CONSOLE_LOGGER_BACKEND_HPP
#define CONSOLE_LOGGER_BACKEND_HPP

#include "../backend/console_log_backend.hpp"
#include "../logger.hpp"
#include "../constants.hpp"
#include <memory> // std::shared_ptr
#include <mutex>  // std::mutex, std::lock_guard

namespace zerg
{

inline std::shared_ptr<Logger<DEFAULT_BUFFER_SIZE>> &getConsoleLogger()
{
    static std::shared_ptr<Logger<DEFAULT_BUFFER_SIZE>> consoleInstance;
    static std::mutex mtx;

    std::lock_guard<std::mutex> lock(mtx);
    if (!consoleInstance)
    {
        auto consoleBackend = std::make_unique<ConsoleLogBackend>();
        consoleInstance = std::make_shared<Logger<DEFAULT_BUFFER_SIZE>>(
            "unused_filename_for_console", Verbosity::DEBUG_LVL, std::move(consoleBackend));
    }
    return consoleInstance;
}


} // namespace zerg
#define cpp_log_console(level, format, ...)                                                        \
    ::zerg::getConsoleLogger()->log(level, __FILE__, __LINE__, format, ##__VA_ARGS__)


#endif // CONSOLE_LOGGER_BACKEND_HPP