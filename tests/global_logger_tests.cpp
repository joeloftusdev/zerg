#include <gtest/gtest.h>
#include "../include/cpp_logger/global_logger.hpp"
#include "test_utils.hpp"
#include <string>
#include <thread>
#include <vector>

void logMessages(const std::string& filename, int thread_id) {
    for (int i = 0; i < 100; ++i) {
        cpp_log_with_file(cpp_logger::Verbosity::INFO_LVL, filename, "Thread %d, message %d", thread_id, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //small delay like realworld example
    }
}


TEST(GlobalLoggerTest, LogWithDifferentFiles) {
    const std::string default_filename = "global_logfile.log";
    const std::string custom_filename = "custom_logfile.log";
    std::ofstream ofs1(default_filename, std::ofstream::out | std::ofstream::trunc);
    ofs1.close();
    std::ofstream ofs2(custom_filename, std::ofstream::out | std::ofstream::trunc);
    ofs2.close();

    cpp_logger::getGlobalLogger(default_filename);

    cpp_log(cpp_logger::Verbosity::INFO_LVL, "Test message with default file");
    cpp_log_with_file(cpp_logger::Verbosity::DEBUG_LVL, custom_filename, "Test message with custom file");

    std::string log_content_default = readFile(default_filename);
    std::string log_content_custom = readFile(custom_filename);

    EXPECT_NE(log_content_default.find("Test message with default file"), std::string::npos);
    EXPECT_NE(log_content_default.find("global_logger_tests.cpp"), std::string::npos);

    EXPECT_NE(log_content_custom.find("Test message with custom file"), std::string::npos);
    EXPECT_NE(log_content_custom.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, LogWithDefaultFile) {
    const std::string default_filename = "global_logfile.log";

    std::ofstream ofs(default_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    cpp_logger::getGlobalLogger(default_filename);
    cpp_log(cpp_logger::Verbosity::INFO_LVL, "Test message with default file");

    std::string log_content = readFile(default_filename);
    EXPECT_NE(log_content.find("Test message with default file"), std::string::npos);
    EXPECT_NE(log_content.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, LogWithCustomFile) {
    const std::string custom_filename = "custom_logfile.log";

    std::ofstream ofs(custom_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    cpp_log_with_file(cpp_logger::Verbosity::INFO_LVL, custom_filename, "Test message with custom file");

    std::string log_content = readFile(custom_filename);
    EXPECT_NE(log_content.find("Test message with custom file"), std::string::npos);
    EXPECT_NE(log_content.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, ThreadSafety) {
    const std::string filename = "thread_safety_logfile.log";
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    const int num_threads = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(logMessages, filename, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::string log_content = readFile(filename);
    int message_count = 0;
    std::istringstream iss(log_content);
    std::string line;
    while (std::getline(iss, line)) {
        ++message_count;
    }

    EXPECT_EQ(message_count, num_threads * 100);
}