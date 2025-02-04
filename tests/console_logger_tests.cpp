#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "zerg/global/console_logger.hpp" // for cpp_log_console, getGlobalConsoleLogger
#include <sstream>
#include <string>

void logConsoleMessages(int thread_id, int message_count)
{
    for (int i = 0; i < message_count; ++i)
    {
        cpp_log_console(zerg::Verbosity::INFO_LVL, "Thread {}, message {}", thread_id, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST(ConsoleLoggerTest, BasicConsoleLog)
{
    testing::internal::CaptureStdout();

    cpp_log_console(zerg::Verbosity::INFO_LVL, "This is a console log test");

    zerg::getConsoleLogger()->sync();
    zerg::getConsoleLogger()->waitUntilEmpty();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("This is a console log test"), std::string::npos)
        << "Expected log message not found in console output: " << output;
}

TEST(ConsoleLoggerTest, ThreadSafety)
{
    // Capture std::cout
    testing::internal::CaptureStdout();

    const int num_threads = 10;
    const int messages_per_thread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(logConsoleMessages, i, messages_per_thread);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    zerg::getConsoleLogger()->sync();
    zerg::getConsoleLogger()->waitUntilEmpty();

    std::string output = testing::internal::GetCapturedStdout();

    int message_count = 0;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line))
    {
        ++message_count;
    }

    int expected_total = num_threads * messages_per_thread;
    EXPECT_GE(message_count, static_cast<int>(0.99 * expected_total))
        << "Missing too many log lines. Expected near " << expected_total << ", got "
        << message_count;
}

TEST(ConsoleLoggerTest, VerbosityLevels)
{
    testing::internal::CaptureStdout();

    // Test all verbosity levels
    cpp_log_console(zerg::Verbosity::DEBUG_LVL, "Debug message: {}", 1);
    cpp_log_console(zerg::Verbosity::INFO_LVL, "Info message: {}", "test");
    cpp_log_console(zerg::Verbosity::WARN_LVL, "Warning message: {:.2f}", 3.141);
    cpp_log_console(zerg::Verbosity::ERROR_LVL, "Error message: {}", true);
    cpp_log_console(zerg::Verbosity::FATAL_LVL, "Fatal message: {}", 'F');

    zerg::getConsoleLogger()->sync();
    zerg::getConsoleLogger()->waitUntilEmpty();

    std::string output = testing::internal::GetCapturedStdout();
    
    // Verify each level appears
    EXPECT_NE(output.find("[DEBUG]"), std::string::npos);
    EXPECT_NE(output.find("[INFO]"), std::string::npos);
    EXPECT_NE(output.find("[WARN]"), std::string::npos);
    EXPECT_NE(output.find("[ERROR]"), std::string::npos);
    EXPECT_NE(output.find("[FATAL]"), std::string::npos);

    // Verify messages appear
    EXPECT_NE(output.find("Debug message"), std::string::npos);
    EXPECT_NE(output.find("Info message"), std::string::npos);
    EXPECT_NE(output.find("Warning message"), std::string::npos);
    EXPECT_NE(output.find("Error message"), std::string::npos);
    EXPECT_NE(output.find("Fatal message"), std::string::npos);

    // Verify arguments appear
    EXPECT_NE(output.find("1"), std::string::npos);
    EXPECT_NE(output.find("test"), std::string::npos);
    EXPECT_NE(output.find("3.14"), std::string::npos);
    EXPECT_NE(output.find("true"), std::string::npos);
    EXPECT_NE(output.find("F"), std::string::npos);
}