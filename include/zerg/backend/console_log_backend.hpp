#ifndef CONSOLE_LOG_BACKEND_HPP
#define CONSOLE_LOG_BACKEND_HPP

#include <unistd.h>         // ::write, STDOUT_FILENO
#include "ilog_backend.hpp" // ILogBackend

namespace zerg
{

class ConsoleLogBackend : public ILogBackend
{
  public:
    ConsoleLogBackend() = default;
    ~ConsoleLogBackend() override = default;

    // Using the posix write call can be faster I think?

    void write(const char *data, std::streamsize size) override
    {
        ::write(STDOUT_FILENO, data, static_cast<size_t>(size)); // write directly to stdout
    }

    void writeNewline() override
    {
        static const char newline = '\n';
        ::write(STDOUT_FILENO, &newline, 1);
    }

    void flush() override
    {
        // there isn’t a user-space buffer to flush, so a separate flush() isn’t needed.
    }
};

} // namespace zerg

#endif // CONSOLE_LOG_BACKEND_HPP