#include <gtest/gtest.h>
#include "../include/cpp_logger/global_logger.hpp"
#include "test_utils.hpp"
#include <string>

TEST(GlobalLoggerTest, LogWithDefaultFile) {
    const std::string default_filename = "global_logfile.log";

    std::ofstream ofs(default_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    cpp_log(cpp_logger::Verbosity::INFO, "Test message with default file");

    std::string log_content = readFile(default_filename);
    EXPECT_NE(log_content.find("Test message with default file"), std::string::npos);
    EXPECT_NE(log_content.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, LogWithCustomFile) {
    const std::string custom_filename = "test_custom_logfile.log";

    std::ofstream ofs(custom_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    cpp_log_with_file(cpp_logger::Verbosity::DEBUG, custom_filename, "Test message with custom file");

    std::string log_content = readFile(custom_filename);
    EXPECT_NE(log_content.find("Test message with custom file"), std::string::npos);
    EXPECT_NE(log_content.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, LogWithDifferentFiles) {
    const std::string default_filename = "test_global_logfile.log";
    const std::string custom_filename = "test_custom_logfile.log";

    std::ofstream ofs1(default_filename, std::ofstream::out | std::ofstream::trunc);
    ofs1.close();
    std::ofstream ofs2(custom_filename, std::ofstream::out | std::ofstream::trunc);
    ofs2.close();


    cpp_logger::getGlobalLogger(default_filename);

    cpp_log(cpp_logger::Verbosity::INFO, "Test message with default file");
    cpp_log_with_file(cpp_logger::Verbosity::DEBUG, custom_filename, "Test message with custom file");

    std::string log_content_default = readFile(default_filename);
    std::string log_content_custom = readFile(custom_filename);

    EXPECT_NE(log_content_default.find("Test message with default file"), std::string::npos);
    EXPECT_NE(log_content_default.find("global_logger_tests.cpp"), std::string::npos);

    EXPECT_NE(log_content_custom.find("Test message with custom file"), std::string::npos);
    EXPECT_NE(log_content_custom.find("global_logger_tests.cpp"), std::string::npos);
}