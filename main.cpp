#include "include/cpp_logger/global_logger.hpp"

//#define LOG(logger, level, format, ...) logger.log(level, __FILE__, __LINE__, format, __VA_ARGS__)

int main() {
    //cpp_logger::Logger<1024, cpp_logger::Verbosity::DEBUG> logger("logfile.log");

    // LOG(logger, cpp_logger::Verbosity::DEBUG, "Debug %.1f message", 1.0);
    // LOG(logger, cpp_logger::Verbosity::INFO, "Info %d message", 2);
    // LOG(logger, cpp_logger::Verbosity::WARN, "Warning %s message", "test");
    // LOG(logger, cpp_logger::Verbosity::ERROR, "Error %c message", 'E');
    // LOG(logger, cpp_logger::Verbosity::FATAL, "Fatal %s message with number %d", "fatal", 5);

    cpp_log(cpp_logger::Verbosity::DEBUG, "Debug %.1f message", 1.0);
    cpp_log(cpp_logger::Verbosity::INFO, "Info %d message", 2);
    cpp_log(cpp_logger::Verbosity::WARN, "Warning %s message", "test");
    cpp_log(cpp_logger::Verbosity::ERROR, "Error %c message", 'E');
    cpp_log(cpp_logger::Verbosity::FATAL, "Fatal %s message with number %d", "fatal", 5);

    // Use the cpp_log_with_file macro to log messages with a specific log file
    cpp_log_with_file(cpp_logger::Verbosity::DEBUG, "custom_logfile.log", "Debug %.1f message", 1.0);

    return 0;
}