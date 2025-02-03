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

#ifndef FILE_LOG_BACKEND_HPP
#define FILE_LOG_BACKEND_HPP

#include <fstream>          // std::ofstream
#include "ilog_backend.hpp" // ILogBackend
#include "../constants.hpp" // DEFAULT_BUFFER_SIZE

namespace cpp_logger
{
class FileLogBackend : public ILogBackend
{
  public:
    explicit FileLogBackend(const std::string &filename)
    {
        _ofs.open(filename, std::ios::out | std::ios::app);
        static char fileBuffer[DEFAULT_BUFFER_SIZE];
        _ofs.rdbuf()->pubsetbuf(fileBuffer, sizeof(fileBuffer));
    }
    ~FileLogBackend() override
    {
        if (_ofs.is_open())
            _ofs.close();
    }
    void write(const char *data, std::streamsize size) override { _ofs.write(data, size); }
    void writeNewline() override { _ofs.put('\n'); }
    void flush() override { _ofs.flush(); }

  private:
    std::ofstream _ofs;
};
} // namespace cpp_logger

#endif // FILE_LOG_BACKEND_HPP
