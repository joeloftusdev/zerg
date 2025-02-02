#include <gtest/gtest.h>
#include "../include/cpp_logger/global_logger.hpp"
#include "test_utils.hpp"
#include <string>
#include <thread>
#include <vector>

void logMessages(const std::string &filename, int thread_id)
{
    for (int i = 0; i < 100; ++i)
    {
        cpp_log_with_file(cpp_logger::Verbosity::INFO_LVL, filename, "Thread {}, message {}",
                          thread_id, i);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); 
    }
}

TEST(GlobalLoggerTest, LogWithDifferentFiles)
{
    const std::string default_filename = "global_logfile.log";
    const std::string custom_filename = "custom_logfile.log";
    std::ofstream ofs1(default_filename, std::ofstream::out | std::ofstream::trunc);
    ofs1.close();
    std::ofstream ofs2(custom_filename, std::ofstream::out | std::ofstream::trunc);
    ofs2.close();

    cpp_log(cpp_logger::Verbosity::INFO_LVL, "Test message with default file");
    cpp_log_with_file(cpp_logger::Verbosity::DEBUG_LVL, custom_filename,
                      "Test message with custom file");

    cpp_logger::getGlobalLogger(default_filename)->sync();    
    cpp_logger::getGlobalLogger(custom_filename)->sync();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::string log_content_default = readFile(default_filename);
    std::string log_content_custom = readFile(custom_filename);

    std::cout << "Default log file content:\n" << log_content_default << std::endl;
    std::cout << "Custom log file content:\n" << log_content_custom << std::endl;

    EXPECT_NE(log_content_default.find("Test message with default file"), std::string::npos);
    EXPECT_NE(log_content_default.find("global_logger_tests.cpp"), std::string::npos);

    EXPECT_NE(log_content_custom.find("Test message with custom file"), std::string::npos);
    EXPECT_NE(log_content_custom.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, LogWithDefaultFile)
{
    const std::string default_filename = "global_logfile.log";

    std::ofstream ofs(default_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    cpp_logger::getGlobalLogger(default_filename);
    cpp_log(cpp_logger::Verbosity::INFO_LVL, "Test message with default file");
    cpp_logger::getGlobalLogger()->sync();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::string log_content = readFile(default_filename);
    EXPECT_NE(log_content.find("Test message with default file"), std::string::npos);
    EXPECT_NE(log_content.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, LogWithCustomFile)
{
    const std::string custom_filename = "custom_logfile.log";

    std::ofstream ofs(custom_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    cpp_log_with_file(cpp_logger::Verbosity::INFO_LVL, custom_filename,
                      "Test message with custom file");
    cpp_logger::getGlobalLogger(custom_filename)->sync();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string log_content = readFile(custom_filename);
    EXPECT_NE(log_content.find("Test message with custom file"), std::string::npos);
    EXPECT_NE(log_content.find("global_logger_tests.cpp"), std::string::npos);
}

TEST(GlobalLoggerTest, ThreadSafety_Resilient)
{
    const std::string filename = "thread_safety_logfile.log";
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    const int num_threads = 10;
    const int messages_per_thread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(logMessages, filename, i);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    std::string log_content = readFile(filename);
    int message_count = 0;
    std::istringstream iss(log_content);
    std::string line;
    while (std::getline(iss, line))
    {
        ++message_count;
    }

    int expected_total = num_threads * messages_per_thread;
    // logger may drop messages to maintain performance,
    // we assert that at least 99% of the messages were logged.
    EXPECT_GE(message_count, static_cast<int>(0.99 * expected_total));
}