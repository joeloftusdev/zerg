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


#ifndef LOCK_FREE_QUEUE_HPP // MPMC lock-free queue
#define LOCK_FREE_QUEUE_HPP

#include <atomic> // std::atomic, std::memory_order_*
#include <vector> // std::vector
#include <cstddef> // std::size_t
#include <array> // std::array
#include "constants.hpp" // CACHE_LINE_SIZE, SHIFT_*
#include "macros.hpp" // PREFETCH, likely, unlikely


template<typename T>
class LockFreeQueue {
    struct alignas(CACHE_LINE_SIZE) AlignedIndex {
        std::atomic<size_t> value{0}; 
        std::atomic<size_t> tag{0};  // ABA protection tag increments on every operation
        std::array<char, CACHE_LINE_SIZE - (2 * sizeof(std::atomic<size_t>))> padding{}; // padding to avoid false sharing
    };

struct Slot {

    alignas(CACHE_LINE_SIZE) std::atomic<size_t> turn{0};
    // "storage' is a raw (untyped) buffer allocated with proper size and alignment for T.
    // the object of type T is constructed in-place in this storage when enqueued.
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
};

public:
    explicit LockFreeQueue(const size_t capacity)
        : _capacity(nextPowerOf2(capacity)) // round up to next power of 2
        , _mask(_capacity - 1) // mask for fast modulo
        , _slots(_capacity) // allocate slots
    {}

    [[nodiscard]] bool enqueue(const T& item) { // copy
        return enqueue_impl(item);
    }


    [[nodiscard]] bool enqueue(T&& item) {  // Move
        return enqueue_impl(std::move(item));
    }

private:
    template<typename U>
    [[nodiscard]] bool enqueue_impl(U&& item) {
        for (;;) {
            size_t head = _head.value.load(std::memory_order_relaxed);
            size_t tail = _tail.value.load(std::memory_order_acquire);
            // Queue is full if the number of enqueued items equals (_capacity - 1)
            if ((head - tail) >= (_capacity - 1)) {
                return false;
            }
            size_t idx = head & _mask;
            size_t turn = head / _capacity;

            PREFETCH(&_slots[idx].turn);
            // Wait until slot.turn == 2*turn (slot empty)
            if (unlikely(_slots[idx].turn.load(std::memory_order_acquire) != 2 * turn)) {
                return false;
            }
            if (_head.value.compare_exchange_weak(
                    head, head + 1,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed))
            {
                // construct item in slo t
                new (&_slots[idx].storage) T(std::forward<U>(item));
                // Mark slot as full
                _slots[idx].turn.store(2 * turn + 1, std::memory_order_release);
                return true;
            }
        }
    }

public:
    [[nodiscard]] bool dequeue(T& item) {
        for (;;) {
            size_t tail = _tail.value.load(std::memory_order_relaxed);
            size_t idx = tail & _mask;
            size_t turn = tail / _capacity;
            // Wait until slot.turn == 2*turn + 1 (meaning full)
            PREFETCH(&_slots[idx].turn);
             if (unlikely(_slots[idx].turn.load(std::memory_order_acquire) != 2 * turn + 1)) {
                // slots not ready
                return false;
            }
            if (_tail.value.compare_exchange_weak(
                    tail, tail + 1,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed))
            {
                // moving out the stored item
                T* ptr = reinterpret_cast<T*>(&_slots[idx].storage);
                item = std::move(*ptr);
                ptr->~T();
                // mark the slot empty
                _slots[idx].turn.store(2 * (turn + 1), std::memory_order_release);
                return true;
            }
        }
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
    // Reserve one slot so the queue appears "full" when one slot remains
        [[nodiscard]]__attribute__((noinline)) bool is_queue_full(const size_t next_head) const {
            // compare "next_head" with one slot behind the tail:
            // i.e. treat the queue as if it has _capacity - 1 usable slots.
            // So if next_head == _tail.value - 1 (modulo mask), it's "full."
            const size_t tail_val = _tail.value.load(std::memory_order_acquire);
            return unlikely(next_head & _mask) == ((tail_val - 1) & _mask);
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
    AlignedIndex _head{};  // producer writes
    AlignedIndex _tail{};  // consumer reads
    std::vector<Slot> _slots; // slots for storing items
};

#endif // LOCK_FREE_QUEUE_HPP

// MPMC lock-free queue