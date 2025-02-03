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


#ifndef LOGGER_TPP
#define LOGGER_TPP

#include "logger.hpp" 


namespace cpp_logger
{

template <std::size_t MaxFileSize, std::size_t BufferSize>
Logger<MaxFileSize, BufferSize>::Logger(std::string filename, const Verbosity logLevel,
                                        std::unique_ptr<ILogBackend> backend)
    : _filename(std::move(filename)),
      _log_level(logLevel),
      _log_buffer(BufferSize),
      _stop_logging(false)
{
    if (!backend)
    {
        backend = std::make_unique<FileLogBackend>(_filename);
    }
    _backend = std::move(backend);
    _logging_thread = std::thread(&Logger::processLogQueue, this);
}

template <std::size_t MaxFileSize, std::size_t BufferSize>
Logger<MaxFileSize, BufferSize>::~Logger()
{
    sync();
    _stop_logging = true;
    _cv.notify_all();
    if (_logging_thread.joinable())
    {
        _logging_thread.join();
    }
}



template <std::size_t MaxFileSize, std::size_t BufferSize>
Logger<MaxFileSize, BufferSize>::Logger(Logger &&other) noexcept
    : _filename(std::move(other._filename)), _backend(std::move(other._backend)),
      _current_size(other._current_size), _log_buffer(std::move(other._log_buffer)),
      _stop_logging(other._stop_logging.load())
    {
        _log_level.store(other._log_level.load());
        other._current_size = 0;
        other._stop_logging = true;
    }

template <std::size_t MaxFileSize, std::size_t BufferSize>
Logger<MaxFileSize, BufferSize>& Logger<MaxFileSize, BufferSize>::operator=(Logger &&other) noexcept
{
        if (this != &other)
        {
            _filename = std::move(other._filename);
            _backend = std::move(other._backend);
            _current_size = other._current_size;
            _log_buffer = std::move(other._log_buffer);
            _stop_logging = other._stop_logging.load();
            _log_level.store(other._log_level.load());
            other._current_size = 0;
            other._stop_logging = true;
        }
        return *this;
    }

template <std::size_t MaxFileSize, std::size_t BufferSize>
void Logger<MaxFileSize, BufferSize>::setLogLevel(Verbosity level)
{
    _log_level.store(level, std::memory_order_relaxed);
}

template <std::size_t MaxFileSize, std::size_t BufferSize>
template <typename... Args>
void Logger<MaxFileSize, BufferSize>::log(Verbosity level, const char *file, int line, const char *format, Args&&... args)
{
    if (likely(level >= _log_level.load(std::memory_order_relaxed)))
    {
        LogEntry entry;
        entry.level = level;
        entry.file = file;
        entry.line = line;
        entry.format = format;
        entry.args = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);

        if (_log_buffer.enqueue(std::move(entry)))
        {
            _cv.notify_one();
        }
    }
}

template <std::size_t MaxFileSize, std::size_t BufferSize>
void Logger<MaxFileSize, BufferSize>::sync()
{
    syncLogs<LogEntry>(
        _log_buffer,
        _backend,
        _file_mutex,
        _empty_cv,
        _empty_mutex,
        [this](const LogEntry &entry) { processLogEntry(entry); }
    );
}

template <std::size_t MaxFileSize, std::size_t BufferSize>
void Logger<MaxFileSize, BufferSize>::waitUntilEmpty()
{
    ::cpp_logger::waitUntilEmpty<LogEntry>(_log_buffer);
}


    /* TODO: implement log rotation in backend not here */ 
    // Maybe add support for logrotate or similar 
    //https://linux.die.net/man/8/logrotate
template <std::size_t MaxFileSize, std::size_t BufferSize>
void Logger<MaxFileSize, BufferSize>::rotateLogFile()
    {
        // 
        _backend = std::make_unique<FileLogBackend>(_filename);
        _current_size = 0;
    }
    /* TODO: implement log rotation in backend not here */

template <std::size_t MaxFileSize, std::size_t BufferSize>
void Logger<MaxFileSize, BufferSize>::processLogQueue()
    {
        std::unique_lock<std::mutex> lock(_log_mutex);
        
        while (!_stop_logging)
        {
            // wait until notified or stopped,  no longer polling
            _cv.wait(lock, [this] { 
                return _stop_logging || !_log_buffer.isEmpty(); 
            });
            
            //assuming stopping t he logging is rare
            if (unlikely(_stop_logging)) break;
                
            std::vector<LogEntry> batch;
            LogEntry entry;
            
        // batch of entries under lock
        while (likely(_log_buffer.dequeue(entry)))
        {
            // prefetch the next entry - minimize cache misses
            PREFETCH(&_log_buffer);
            batch.push_back(std::move(entry));
        }
            // process batch without lock
            lock.unlock();
            for (const auto& batch_entry : batch) {
                processLogEntry(batch_entry);
            }
            lock.lock();
        }
    }

template <std::size_t MaxFileSize, std::size_t BufferSize>
void Logger<MaxFileSize, BufferSize>::processLogEntry(const LogEntry &entry)
{
        fmt::memory_buffer log_entry_buffer; // buffer to hold the formatted log entry
        // now format log entry directly into the buffer no string
        fmt::format_to(std::back_inserter(log_entry_buffer), "{} [{}] {}:{} {}", getCurrentTime(), getVerbosityString(entry.level),
                    getFileName(entry.file), entry.line, entry.args);

        sanitizeString(log_entry_buffer);

        // protect file ops with a mutex
        std::lock_guard<std::mutex> lock(_file_mutex);

        if (_current_size + log_entry_buffer.size() > MaxFileSize)
        {
            rotateLogFile();
        }
        _backend->write(log_entry_buffer.data(), static_cast<std::streamsize>(log_entry_buffer.size()));
        _backend->writeNewline();
        _current_size += log_entry_buffer.size();
    }

template <std::size_t MaxFileSize, std::size_t BufferSize>
std::string Logger<MaxFileSize, BufferSize>::getCurrentTime()
{
        struct timespec ts;
        // Using clock_gettime w/ CLOCK_REALTIME_COARSE (linux) for a faster timestamp
        // https://www.man7.org/linux/man-pages/man3/clock_gettime.3.html
        clock_gettime(CLOCK_REALTIME_COARSE, &ts);
        std::tm tm_time{};
        localtime_r(&ts.tv_sec, &tm_time);
        char buffer[32];
        // std::strftime to format the time into the fixed-size buffer. 
        // Reduced allocations I think?
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", &tm_time);
        return buffer;
    }

template <std::size_t MaxFileSize, std::size_t BufferSize>
std::string Logger<MaxFileSize, BufferSize>::getVerbosityString(const Verbosity level)
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

template <std::size_t MaxFileSize, std::size_t BufferSize>
std::string Logger<MaxFileSize, BufferSize>::getFileName(const std::string &path)
{
        const size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }

template <std::size_t MaxFileSize, std::size_t BufferSize>
void Logger<MaxFileSize, BufferSize>::sanitizeString(fmt::memory_buffer &buffer)
{
        //remove non-printable characters from the buffer
        auto it = std::remove_if(buffer.begin(), buffer.end(),
                                [](unsigned char c) { return std::isprint(c) == 0; });
        // resize to remove the unwanted characters
        buffer.resize(it - buffer.begin());

    }
}; //namespace cpp_logger


#endif // LOGGER_TPP