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

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef> // std::size_t

constexpr size_t DEFAULT_BUFFER_SIZE = 1024 * 1024;
constexpr size_t MAX_FILE_SIZE = 1024;

constexpr size_t CACHE_LINE_SIZE = 64;

constexpr size_t SHIFT_1 = 1;   
constexpr size_t SHIFT_2 = 2;   
constexpr size_t SHIFT_4 = 4; 
constexpr size_t SHIFT_8 = 8;
constexpr size_t SHIFT_16 = 16;
constexpr size_t SHIFT_32 = 32;

#endif // CONSTANTS_HPP