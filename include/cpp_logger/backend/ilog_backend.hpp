#ifndef ILOG_BACKEND_HPP
#define ILOG_BACKEND_HPP

#include <ios> // std::streamsize

namespace cpp_logger
{

// Interface for log backends
// TODO Multiple backends i.e console, network
class ILogBackend {
public:
    virtual ~ILogBackend() = default;
    virtual void write(const char* data, std::streamsize size) = 0;
    virtual void writeNewline() = 0;
    virtual void flush() = 0;
};
}

#endif // ILOG_BACKEND_HPP



