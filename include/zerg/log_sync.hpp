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

#ifndef LOG_SYNCER_HPP
#define LOG_SYNCER_HPP

#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include "lock_free_queue.hpp"
#include "backend/ilog_backend.hpp"

namespace zerg
{

template <typename LogEntry>
void syncLogs(LockFreeQueue<LogEntry> &log_buffer, std::unique_ptr<ILogBackend> &backend,
              std::mutex &file_mutex, std::condition_variable &empty_cv, std::mutex &empty_mutex,
              std::function<void(const LogEntry &)> processLogEntry)
{
#ifdef BENCHMARK_MODE
    LogEntry entry;
    while (log_buffer.dequeue(entry))
    {
        processLogEntry(entry);
    }
    {
        std::lock_guard<std::mutex> lock(file_mutex);
        backend->flush();
    }
#else
    LogEntry entry;
    const auto stable_duration = std::chrono::milliseconds(50);
    auto start_stable = std::chrono::steady_clock::now();

    while (true)
    {
        bool processed = false;
        while (log_buffer.dequeue(entry))
        {
            processLogEntry(entry);
            processed = true;
        }
        {
            std::lock_guard<std::mutex> lock(file_mutex);
            backend->flush();
        }
        if (processed)
        {
            start_stable = std::chrono::steady_clock::now();
        }
        else
        {
            if (std::chrono::steady_clock::now() - start_stable >= stable_duration)
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    {
        std::lock_guard<std::mutex> lock(empty_mutex);
        empty_cv.notify_all();
    }
#endif
}

template <typename LogEntry> void waitUntilEmpty(LockFreeQueue<LogEntry> &log_buffer)
{
    const auto timeout = std::chrono::milliseconds(500);
    auto start = std::chrono::steady_clock::now();
    while (!log_buffer.isEmpty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (std::chrono::steady_clock::now() - start > timeout)
            break;
    }
}

} // namespace zerg

#endif // LOG_SYNCER_HPP