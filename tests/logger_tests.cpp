#include <gtest/gtest.h>
#include "../include/zerg/logger.hpp"
#include "test_utils.hpp"
#include <string>

#define LOG_TEST(logger, level, ...) logger.log(level, __FILE__, __LINE__, __VA_ARGS__)

TEST(LoggerTest, LogSingleMessage)
{
    const std::string filename = "test_log.log";
    zerg::Logger<1024> logger(filename, zerg::Verbosity::DEBUG_LVL);

    LOG_TEST(logger, zerg::Verbosity::DEBUG_LVL, "Test message");

    logger.sync();
    logger.waitUntilEmpty();

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("Test message"), std::string::npos);
    EXPECT_NE(log_content.find("logger_tests.cpp"), std::string::npos);
}

TEST(LoggerTest, LogMultipleMessages)
{
    const std::string filename = "test_log.log";
    zerg::Logger<1024> logger(filename, zerg::Verbosity::DEBUG_LVL);

    LOG_TEST(logger, zerg::Verbosity::DEBUG_LVL, "First message");
    LOG_TEST(logger, zerg::Verbosity::DEBUG_LVL, "Second message");

    logger.sync();
    logger.waitUntilEmpty();

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("First message"), std::string::npos);
    EXPECT_NE(log_content.find("Second message"), std::string::npos);
    EXPECT_NE(log_content.find("logger_tests.cpp"), std::string::npos);
}

// Make this actually test the rotation
TEST(LoggerTest, RotateLogFile)
{
    const std::string filename = "test_log.log";
    zerg::Logger<100> logger(
        filename, zerg::Verbosity::DEBUG_LVL); // small max file size to trigger rotation

    LOG_TEST(logger, zerg::Verbosity::DEBUG_LVL, "Message 1");

    logger.sync();
    logger.waitUntilEmpty();

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("Message 1"), std::string::npos);
}

TEST(LoggerTest, LogWithDifferentVerbosityLevels)
{
    const std::string filename = "test_log.log";
    zerg::Logger<1024> logger(filename, zerg::Verbosity::WARN_LVL);

    LOG_TEST(logger, zerg::Verbosity::DEBUG_LVL, "Debug message");
    LOG_TEST(logger, zerg::Verbosity::INFO_LVL, "Info message");
    LOG_TEST(logger, zerg::Verbosity::WARN_LVL, "Warning message");
    LOG_TEST(logger, zerg::Verbosity::ERROR_LVL, "Error message");

    logger.sync();
    logger.waitUntilEmpty();

    std::string log_content = readFile(filename);
    EXPECT_EQ(log_content.find("Debug message"), std::string::npos);
    EXPECT_EQ(log_content.find("Info message"), std::string::npos);
    EXPECT_NE(log_content.find("Warning message"), std::string::npos);
    EXPECT_NE(log_content.find("Error message"), std::string::npos);
    EXPECT_NE(log_content.find("logger_tests.cpp"), std::string::npos);
}

TEST(LoggerTest, LogFormattedMessages)
{
    const std::string filename = "test_log.log";
    zerg::Logger<1024> logger(filename, zerg::Verbosity::DEBUG_LVL);

    LOG_TEST(logger, zerg::Verbosity::DEBUG_LVL, "Debug {:.1f} message", 1.0);
    LOG_TEST(logger, zerg::Verbosity::INFO_LVL, "Info {} message", 2);
    LOG_TEST(logger, zerg::Verbosity::WARN_LVL, "Warning {} message", "test");
    LOG_TEST(logger, zerg::Verbosity::ERROR_LVL, "Error {} message", 'E');
    LOG_TEST(logger, zerg::Verbosity::FATAL_LVL, "Fatal {} message with number {}", "fatal",
             5);

    logger.sync();
    logger.waitUntilEmpty();

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("Debug 1.0 message"), std::string::npos);
    EXPECT_NE(log_content.find("Info 2 message"), std::string::npos);
    EXPECT_NE(log_content.find("Warning test message"), std::string::npos);
    EXPECT_NE(log_content.find("Error E message"), std::string::npos);
    EXPECT_NE(log_content.find("Fatal fatal message with number 5"), std::string::npos);
}

TEST(LoggerTest, SanitizeNonPrintableCharacters)
{
    const std::string filename = "test_log.log";
    zerg::Logger<1024> logger(filename, zerg::Verbosity::DEBUG_LVL);

    LOG_TEST(logger, zerg::Verbosity::DEBUG_LVL,
             "Test message with non-printable \x01\x02\x03 characters");

    logger.sync();
    logger.waitUntilEmpty();

    std::string log_content = readFile(filename);
    EXPECT_NE(log_content.find("Test message with non-printable  characters"), std::string::npos);
    EXPECT_EQ(log_content.find("\x01"), std::string::npos);
    EXPECT_EQ(log_content.find("\x02"), std::string::npos);
    EXPECT_EQ(log_content.find("\x03"), std::string::npos);
}