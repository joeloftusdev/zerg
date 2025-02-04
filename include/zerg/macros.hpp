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

#ifndef MACROS_HPP
#define MACROS_HPP

#if defined(__GNUC__) || defined(__clang__)
#include <xmmintrin.h> // _mm_prefetch
#define PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_T0)
// branch predictor
// __builtin_expect tells compiler which branch is more common
// https://stackoverflow.com/questions/109710/likely-unlikely-macros-in-the-linux-kernel
// This is easier in 20 [[likely]] and [[unlikely]] attributes
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define PREFETCH(addr)
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#endif // MACROS_HPP