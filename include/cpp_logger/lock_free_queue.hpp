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

#include <atomic> // std::atomic, std::memory_order_*
#include <vector> // std::vector
#include <cstddef> // std::size_t
#include <array> // std::array
#include "constants.hpp" // CACHE_LINE_SIZE, SHIFT_*


// prefetching
// __builtin_prefetch tells the compiler to prefetch the data at the address
#if defined(__GNUC__) || defined(__clang__)
    #include <xmmintrin.h>
    // Using static_cast over reinterpret_cast to avoid undefined behavior
    #define PREFETCH(addr) _mm_prefetch(\
        static_cast<const volatile char*>(static_cast<const volatile void*>(addr)),\
        _MM_HINT_T0)
#else
    #define PREFETCH(addr)
#endif


// branch predictor
// __builtin_expect tells compiler which branch is more common
// https://stackoverflow.com/questions/109710/likely-unlikely-macros-in-the-linux-kernel
#if defined(__GNUC__) || defined(__clang__)
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif



template<typename T>
class LockFreeQueue {
    struct alignas(CACHE_LINE_SIZE) AlignedIndex {
        std::atomic<size_t> value{0}; 
        std::atomic<size_t> tag{0};  // ABA protection tag increments on every operation
        std::array<char, CACHE_LINE_SIZE - 2 * sizeof(std::atomic<size_t>)> padding{}; // padding to avoid false sharing
    };

public:
    explicit LockFreeQueue(size_t capacity) 
        : _capacity(nextPowerOf2(capacity)) // round up to next power of 2
        , _mask(_capacity - 1) // mask for fast modulo
        , _buffer(_capacity) // allocate buffer
    {}

    [[nodiscard]] bool enqueue(const T& item) { // copy
        return enqueue_impl(item);
    }


    [[nodiscard]] bool enqueue(T&& item) {  // Move
        return enqueue_impl(std::forward<T>(item)); 
    }

private:
    template<typename U>
    [[nodiscard]] bool enqueue_impl(U&& item) {
        const size_t current_head = _head.value.load(std::memory_order_relaxed); 
        const size_t next_head = (current_head + 1) & _mask; 

        // prefetch next write location to minimize cache misses
        PREFETCH(&_buffer[(next_head + 1) & _mask]);
        
        if (is_queue_full(next_head)) { 
            return false;
        }


        _buffer[current_head] = std::forward<U>(item); 
        _head.value.store(next_head, std::memory_order_release); 
        _head.tag.fetch_add(1, std::memory_order_release); // increment ABA protection tag
        return true;
    }

public:
    [[nodiscard]] bool dequeue(T& item) {
        const size_t current_tail = _tail.value.load(std::memory_order_relaxed); 
        
        // prefetch next read location to minimize cache misses
        PREFETCH(&_buffer[(current_tail + 1) & _mask]);
        
        // check if queue is empty(likely)
        if (likely(current_tail == _head.value.load(std::memory_order_acquire))) { 
            return false;
        }

        item = _buffer[current_tail];
        _tail.value.store((current_tail + 1) & _mask, std::memory_order_release);
        _tail.tag.fetch_add(1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] size_t capacity() const { return _capacity; } // get capacity of queue

    [[nodiscard]] bool isEmpty() const { // check if queue is empty
        return _head.value.load(std::memory_order_relaxed) == 
               _tail.value.load(std::memory_order_relaxed);
    }

    [[nodiscard]] size_t size() const { // check size of queue
        const size_t head = _head.value.load(std::memory_order_relaxed);
        const size_t tail = _tail.value.load(std::memory_order_relaxed);
        return head >= tail ? head - tail : _capacity - (tail - head);
    }
    

private:

    #if defined(__GNUC__) || defined(__clang__)
    // using noinline prevents this cold path from being inlined into the hot path
    // This helps keep the instruction cache efficient for the likely case
    // The Full queue check is rare, so we optimize for the non-full case
    // https://youtu.be/BxfT9fiUsZ4?si=-eG1zfxhSkp3qUsI
        [[nodiscard]]__attribute__((noinline)) bool is_queue_full(const size_t next_head) const {
            // check if queue is full (unlikely)
            return unlikely(next_head == _tail.value.load(std::memory_order_acquire));
        }
    #else
        bool is_queue_full(const size_t next_head) const {
            return next_head == _tail.value.load(std::memory_order_acquire);
        }
    #endif

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

    const size_t _capacity; // total capacity of the queue(powers of 2)
    const size_t _mask; // bitmask for index wrapping
    std::vector<T> _buffer; // circular buffer storage
    AlignedIndex _head{};  // producer writes
    AlignedIndex _tail{};  // consumer reads
};

#endif // LOCK_FREE_QUEUE_HPP