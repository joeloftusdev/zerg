#ifndef FILE_LOG_BACKEND_HPP
#define FILE_LOG_BACKEND_HPP

#include <fstream> // std::ofstream
#include "ilog_backend.hpp" // ILogBackend
#include "../constants.hpp" // DEFAULT_BUFFER_SIZE

namespace cpp_logger
{
class FileLogBackend : public ILogBackend {
public:
    explicit FileLogBackend(const std::string &filename)
    {
        _ofs.open(filename, std::ios::out | std::ios::app);
        static char fileBuffer[DEFAULT_BUFFER_SIZE];
        _ofs.rdbuf()->pubsetbuf(fileBuffer, sizeof(fileBuffer));
    }
    ~FileLogBackend() override { if (_ofs.is_open()) _ofs.close(); }
    void write(const char* data, std::streamsize size) override {
         _ofs.write(data, size);
    }
    void writeNewline() override {
         _ofs.put('\n');
    }
    void flush() override {
         _ofs.flush();
    }
private:
    std::ofstream _ofs;
};
}

#endif // FILE_LOG_BACKEND_HPP



