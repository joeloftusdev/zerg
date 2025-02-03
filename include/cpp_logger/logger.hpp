// Copyright 2025 Joseph A. Loftus
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "constants.hpp"                // MAX_FILE_SIZE, DEFAULT_BUFFER_SIZE
#include "lock_free_queue.hpp"          // LockFreeQueue
#include "backend/file_log_backend.hpp" // FileLogBackend
#include "verbosity.hpp"                // Verbosity
#include "log_sync.hpp"                 // syncLogs, waitUntilEmpty

#include <iostream>           // std::cout, std::cerr
#include <fstream>            // std::ofstream
#include <string>             // std::string
#include <time.h>             // for clock_gettime, timespec
#include <ctime>              // std::localtime_r
#include <iomanip>            // std::put_time
#include <fmt/core.h>         // fmt::format
#include <fmt/ostream.h>      // fmt::ostream_formatter
#include <thread>             // std::thread
#include <condition_variable> // std::condition_variable
#include <atomic>             // std::atomic, std::memory_order_*
#include <vector>             // std::vector
#include <array>              // std::array
#include "macros.hpp"         // PREFETCH, likely, unlikely

namespace cpp_logger
{

/*
 * Logger Class Features:
 * 1. Asynchronous Logging: Background thread processes entries for improved performance
 * @processLogQueue
 * 2. Lock-Free Queue: Thread-safe queue with MPMC for efficient passing of log entries @_log_buffer
 * 3. Event-Driven: Uses condition variable for immediate notification of new entries @_cv
 * 4. Batched Processing: Groups log entries to reduce I/O operations and lock contention
 * 5. Safe Shutdown: Ensures all pending logs are written before destruction @sync
 * 6. Thread-Safe: File operations protected by mutex, queue operations lock-free
 * 7. File Rotation: Automatic log file rotation when size limit reached @rotateLogFile
 */

template <std::size_t MaxFileSize, std::size_t BufferSize = MAX_FILE_SIZE> class Logger
{
  public:
    explicit Logger(std::string filename, const Verbosity logLevel = Verbosity::DEBUG_LVL,
                    std::unique_ptr<ILogBackend> backend = nullptr);
    ~Logger();

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&other) noexcept;
    Logger &operator=(Logger &&other) noexcept;

    void setLogLevel(Verbosity level);

    template <typename... Args>
    void log(Verbosity level, const char *file, int line, const char *format, Args &&...args);

    void sync();
    void waitUntilEmpty();

  private:
    struct LogEntry
    {
        Verbosity level{};
        const char *file{};
        int line{};
        const char *format{};
        std::string args;
    };

    std::string _filename;
    std::unique_ptr<ILogBackend> _backend;
    std::size_t _current_size{};
    std::atomic<Verbosity> _log_level{};
    LockFreeQueue<LogEntry> _log_buffer;
    std::condition_variable _cv;
    std::thread _logging_thread;
    std::atomic<bool> _stop_logging;
    std::mutex _log_mutex;
    mutable std::mutex _file_mutex;
    std::condition_variable _empty_cv;
    std::mutex _empty_mutex;

    void rotateLogFile();
    void processLogQueue();
    void processLogEntry(const LogEntry &entry);
    static std::string getCurrentTime();
    static std::string getVerbosityString(const Verbosity level);
    static std::string getFileName(const std::string &path);
    void sanitizeString(fmt::memory_buffer &buffer);
};

} // namespace cpp_logger

#include "logger.tpp"
#endif // LOGGER_HPP