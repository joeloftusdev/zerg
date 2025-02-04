#include "include/zerg/global_logger.hpp"

//#define LOG(logger, level, format, ...) logger.log(level, __FILE__, __LINE__, format, __VA_ARGS__)

int main()
{
    zerg::loadConfiguration("cpp_logger.cfg");

    cpp_log(zerg::Verbosity::DEBUG_LVL, "Debug {:.1f} message", 1.0);
    cpp_log(zerg::Verbosity::INFO_LVL, "Info {} message", 2);
    cpp_log(zerg::Verbosity::WARN_LVL, "Warning {} message", "test");
    cpp_log(zerg::Verbosity::ERROR_LVL, "Error {} message", 'E');
    cpp_log(zerg::Verbosity::FATAL_LVL, "Fatal {} message with number {}", "fatal", 5);

    // Use the cpp_log_with_file macro to log messages with a specific log file
    cpp_log_with_file(zerg::Verbosity::INFO_LVL, "custom_logfile.log", "Debug {:.1f} message",
                      1.0);

    return 0;
}