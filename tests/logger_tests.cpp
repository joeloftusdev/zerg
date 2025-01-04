#include <gtest/gtest.h>
#include "../include/cpp_logger/logger.hpp"
#include "test_utils.hpp"
#include <string>

#define LOG_TEST(logger, level, ...) logger.log(level, __FILE__, __LINE__, __VA_ARGS__)

TEST(LoggerTest, LogSingleMessage) {
    const std::string filename = "test_log.log";
    cpp_logger::Logger<1024, cpp_logger::Verbosity::DEBUG> logger(filename);

    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "Test message");

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("Test message"), std::string::npos);
    EXPECT_NE(log_content.find("logger_tests.cpp"), std::string::npos);
}

TEST(LoggerTest, LogMultipleMessages) {
    const std::string filename = "test_log.log";
    cpp_logger::Logger<1024, cpp_logger::Verbosity::DEBUG> logger(filename);

    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "First message");
    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "Second message");

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("First message"), std::string::npos);
    EXPECT_NE(log_content.find("Second message"), std::string::npos);
    EXPECT_NE(log_content.find("logger_tests.cpp"), std::string::npos);
}

TEST(LoggerTest, RotateLogFile) {
    const std::string filename = "test_log.log";
    cpp_logger::Logger<50, cpp_logger::Verbosity::DEBUG> logger(filename); // small max file size to trigger rotation

    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "Message 1");
    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "Message 2");
    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "Message 3");

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("Message 1"), std::string::npos);
    EXPECT_NE(log_content.find("Message 2"), std::string::npos);
    EXPECT_NE(log_content.find("Message 3"), std::string::npos);
}

TEST(LoggerTest, LogWithDifferentVerbosityLevels) {
    const std::string filename = "test_log.log";
    cpp_logger::Logger<1024, cpp_logger::Verbosity::WARN> logger(filename);

    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "Debug message");
    LOG_TEST(logger, cpp_logger::Verbosity::INFO, "Info message");
    LOG_TEST(logger, cpp_logger::Verbosity::WARN, "Warning message");
    LOG_TEST(logger, cpp_logger::Verbosity::ERROR, "Error message");

    std::string log_content = readFile(filename);
    EXPECT_EQ(log_content.find("Debug message"), std::string::npos);
    EXPECT_EQ(log_content.find("Info message"), std::string::npos);
    EXPECT_NE(log_content.find("Warning message"), std::string::npos);
    EXPECT_NE(log_content.find("Error message"), std::string::npos);
    EXPECT_NE(log_content.find("logger_tests.cpp"), std::string::npos);
}

TEST(LoggerTest, LogFormattedMessages) {
    const std::string filename = "test_log.log";
    cpp_logger::Logger<1024, cpp_logger::Verbosity::DEBUG> logger(filename);

    LOG_TEST(logger, cpp_logger::Verbosity::DEBUG, "Debug %.1f message", 1.0);
    LOG_TEST(logger, cpp_logger::Verbosity::INFO, "Info %d message", 2);
    LOG_TEST(logger, cpp_logger::Verbosity::WARN, "Warning %s message", "test");
    LOG_TEST(logger, cpp_logger::Verbosity::ERROR, "Error %c message", 'E');
    LOG_TEST(logger, cpp_logger::Verbosity::FATAL, "Fatal %s message with number %d", "fatal", 5);

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("Debug 1.0 message"), std::string::npos);
    EXPECT_NE(log_content.find("Info 2 message"), std::string::npos);
    EXPECT_NE(log_content.find("Warning test message"), std::string::npos);
    EXPECT_NE(log_content.find("Error E message"), std::string::npos);
    EXPECT_NE(log_content.find("Fatal fatal message with number 5"), std::string::npos);
}