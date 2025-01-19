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

#include "constants.hpp"
#include "lock_free_queue.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <array>

namespace cpp_logger
{

enum class Verbosity
{
    DEBUG_LVL,
    INFO_LVL,
    WARN_LVL,
    ERROR_LVL,
    FATAL_LVL
};

/*
 * Logger Class Features:
 * 1. Asynchronous Logging: Background thread processes entries for improved performance @processLogQueue
 * 2. Lock-Free Queue: Thread-safe queue with SPSC optimization for efficient passing of log entries @_log_buffer
 * 3. Event-Driven: Uses condition variable for immediate notification of new entries @_cv
 * 4. Batched Processing: Groups log entries to reduce I/O operations and lock contention
 * 5. Safe Shutdown: Ensures all pending logs are written before destruction @sync
 * 6. Thread-Safe: File operations protected by mutex, queue operations lock-free
 * 7. File Rotation: Automatic log file rotation when size limit reached @rotateLogFile
 */

template <std::size_t MaxFileSize, std::size_t BufferSize = MAX_FILE_SIZE>
class Logger
{
public:
    explicit Logger(std::string filename, Verbosity logLevel = Verbosity::DEBUG_LVL)
        : _filename(std::move(filename)), _log_level(logLevel), _stop_logging(false),
          _log_buffer(BufferSize)
    {
        openLogFile();
        _logging_thread = std::thread(&Logger::processLogQueue, this);
    }

    ~Logger()
    {
        sync();
        _stop_logging = true;
        _cv.notify_all();
        if (_logging_thread.joinable())
        {
            _logging_thread.join();
        }
        if (_logfile.is_open())
        {
            _logfile.close();
        }
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    Logger(Logger &&other) noexcept
        : _filename(std::move(other._filename)), _logfile(std::move(other._logfile)),
          _current_size(other._current_size), _log_level(other._log_level),
          _stop_logging(other._stop_logging.load()), _log_buffer(std::move(other._log_buffer))
    {
        other._current_size = 0;
        other._stop_logging = true;
    }

    Logger &operator=(Logger &&other) noexcept
    {
        if (this != &other)
        {
            _filename = std::move(other._filename);
            _logfile = std::move(other._logfile);
            _current_size = other._current_size;
            _log_level = other._log_level;
            _stop_logging = other._stop_logging.load();
            _log_buffer = std::move(other._log_buffer);
            other._current_size = 0;
            other._stop_logging = true;
        }
        return *this;
    }

    void setLogLevel(Verbosity level)
    {
        _log_level.store(level, std::memory_order_relaxed);
    }

    template <typename... Args>
    void log(Verbosity level, const char *file, int line, const char *format, Args &&...args)
    {
        if (level >= _log_level.load(std::memory_order_relaxed))
        {
            // format  message outside of the critical section
            LogEntry entry;
            entry.level = level;
            entry.file = file;
            entry.line = line;
            entry.format = format;
            entry.args = {fmt::format(fmt::runtime(format), std::forward<Args>(args)...)};

            {
                std::lock_guard<std::mutex> lock(_log_mutex); // lock only for enqueue
                if (_log_buffer.enqueue(entry))
                {
                    _cv.notify_one();
                }
            }
        }
    }

    void sync()
    {
        LogEntry entry;
        while (_log_buffer.dequeue(entry))
        {
            processLogEntry(entry);
        }
        std::lock_guard<std::mutex> lock(_file_mutex);
        _logfile.flush();
    }

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
    std::ofstream _logfile;
    std::size_t _current_size{};
    std::atomic<Verbosity> _log_level;
    LockFreeQueue<LogEntry> _log_buffer;
    std::condition_variable _cv;
    std::thread _logging_thread;
    std::atomic<bool> _stop_logging;
    std::mutex _log_mutex;
    mutable std::mutex _file_mutex;

    void openLogFile()
    {
        _logfile.open(_filename, std::ios::out | std::ios::app);
        _current_size = _logfile.tellp();
    }

    void rotateLogFile()
    {
        if (_logfile.is_open())
        {
            _logfile.close();
        }
        _logfile.open(_filename, std::ios::out | std::ios::trunc);
        _current_size = 0;
    }

    // void processLogQueue()
    // {
    //     std::unique_lock<std::mutex> lock(_log_mutex);
        
    //     while (!_stop_logging)
    //     {
    //         // still polling but much better than before
    //         _cv.wait_for(lock, std::chrono::milliseconds(100), 
    //             [this] { return _stop_logging || !_log_buffer.isEmpty(); });
                
    //         lock.unlock(); 
            
    //         LogEntry entry;
    //         while (_log_buffer.dequeue(entry))
    //         {
    //             processLogEntry(entry);
    //         }
            
    //         lock.lock(); 
    //     }
        
    //     lock.unlock();
    //     LogEntry entry;
    //     while (_log_buffer.dequeue(entry))
    //     {
    //         processLogEntry(entry); 
    //     }
    // }

    void processLogQueue()
    {
        std::unique_lock<std::mutex> lock(_log_mutex);
        
        while (!_stop_logging)
        {
            // wait until notified or stopped,  no longer polling
            _cv.wait(lock, [this] { 
                return _stop_logging || !_log_buffer.isEmpty(); 
            });
            
            if (_stop_logging) break;
                
            std::vector<LogEntry> batch;
            LogEntry entry;
            
            // batch of entries under lock
            while (_log_buffer.dequeue(entry)) {
                batch.push_back(std::move(entry));
            }
            
            // process batch without lock
            lock.unlock();
            for (const auto& entry : batch) {
                processLogEntry(entry);
            }
            lock.lock();
        }
    }

    void processLogEntry(const LogEntry &entry)
    {
        // format log entry wth fmt
        std::string log_entry = fmt::format("{} [{}] {}:{} {}",
        getCurrentTime(), getVerbosityString(entry.level),
        getFileName(entry.file), entry.line, entry.args);

        
        sanitizeString(log_entry);

        // Im now protecting file operations with a mutex
        std::lock_guard<std::mutex> lock(_file_mutex);
        
        if (_current_size + log_entry.size() > MaxFileSize)
        {
            rotateLogFile();
        }
        _logfile << log_entry << std::endl;
        _current_size += log_entry.size();
    }

    std::string getCurrentTime() const
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm buf{};
        localtime_r(&in_time_t, &buf);
        std::ostringstream stream;
        stream << std::put_time(&buf, "%Y-%m-%d %X");
        return stream.str();
    }

    std::string getVerbosityString(Verbosity level) const
    {
        switch (level)
        {
        case Verbosity::DEBUG_LVL:
            return "DEBUG";
        case Verbosity::INFO_LVL:
            return "INFO";
        case Verbosity::WARN_LVL:
            return "WARN";
        case Verbosity::ERROR_LVL:
            return "ERROR";
        case Verbosity::FATAL_LVL:
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }

    std::string getFileName(const std::string &path) const
    {
        size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }

    void sanitizeString(std::string &str)
    {
        str.erase(std::remove_if(str.begin(), str.end(),
                                 [](unsigned char c) { return std::isprint(c) == 0; }),
                  str.end());
    }
};

} // namespace cpp_logger

#endif // LOGGER_HPP