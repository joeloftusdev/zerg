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


#ifndef LOCK_FREE_QUEUE_HPP
#define LOCK_FREE_QUEUE_HPP

#include <atomic>
#include <vector>
#include <cstddef>
#include <array>

// cache line size on most x86 processors
constexpr size_t CACHE_LINE_SIZE = 64;

//TODO add support for ARM processors

// #if defined(__x86_64__) || defined(_M_X64)
//     constexpr size_t CACHE_LINE_SIZE = 64;
// #elif defined(__aarch64__) || defined(__arm__)
//     constexpr size_t CACHE_LINE_SIZE = 128;  
// #else
//     constexpr size_t CACHE_LINE_SIZE = 64;   
// #endif

//std::hardware_destructive_interference_size could do this but I want compatibility with C++17 compilers

constexpr size_t SHIFT_1 = 1;   
constexpr size_t SHIFT_2 = 2;   
constexpr size_t SHIFT_4 = 4; 
constexpr size_t SHIFT_8 = 8;
constexpr size_t SHIFT_16 = 16;
constexpr size_t SHIFT_32 = 32;

template<typename T>
class LockFreeQueue {
    struct alignas(CACHE_LINE_SIZE) AlignedIndex {
        std::atomic<size_t> value{0};
        std::array<char, CACHE_LINE_SIZE - sizeof(std::atomic<size_t>)> padding{};
    };

public:
    explicit LockFreeQueue(size_t capacity) 
        : _capacity(nextPowerOf2(capacity))
        , _mask(_capacity - 1)
        , _buffer(_capacity)
    {}

    [[nodiscard]] bool enqueue(const T& item) {
        const size_t current_head = _head.value.load(std::memory_order_relaxed);
        const size_t next_head = (current_head + 1) & _mask;
        
        if (next_head == _tail.value.load(std::memory_order_acquire)) {
            return false; 
        }

        _buffer[current_head] = item;
        _head.value.store(next_head, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool dequeue(T& item) {
        const size_t current_tail = _tail.value.load(std::memory_order_relaxed);
        
        if (current_tail == _head.value.load(std::memory_order_acquire)) {
            return false; 
        }

        item = _buffer[current_tail];
        _tail.value.store((current_tail + 1) & _mask, std::memory_order_release);
        return true;
    }

    [[nodiscard]] size_t capacity() const { return _capacity; }

    [[nodiscard]] bool isEmpty() const {
        return _head.value.load(std::memory_order_acquire) == 
               _tail.value.load(std::memory_order_acquire);
    }
    

private:
    // round up to next power of 2
    static size_t nextPowerOf2(size_t v) {
        v--;
        v |= v >> SHIFT_1;
        v |= v >> SHIFT_2;
        v |= v >> SHIFT_4;
        v |= v >> SHIFT_8;
        v |= v >> SHIFT_16;
        v |= v >> SHIFT_32;
        v++;
        return v;
    }

    const size_t _capacity;
    const size_t _mask;
    std::vector<T> _buffer;
    AlignedIndex _head{};  // producer writes
    AlignedIndex _tail{};  // consumer reads
};

#endif // LOCK_FREE_QUEUE_HPP