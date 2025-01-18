#ifndef LOCK_FREE_QUEUE_HPP
#define LOCK_FREE_QUEUE_HPP

#include <atomic>
#include <vector>

template <typename T>
class LockFreeQueue
{
public:
    explicit LockFreeQueue(std::size_t capacity)
        : _capacity(capacity), _buffer(capacity), _head(0), _tail(0) {}

    bool enqueue(const T &item)
    {
        std::size_t head = _head.load(std::memory_order_relaxed);
        std::size_t next_head = (head + 1) % _capacity;
        if (next_head != _tail.load(std::memory_order_acquire))
        {
            _buffer[head] = item;
            _head.store(next_head, std::memory_order_release);
            return true;
        }
        return false; 
    }

    bool dequeue(T &item)
    {
        std::size_t tail = _tail.load(std::memory_order_relaxed);
        if (tail == _head.load(std::memory_order_acquire))
        {
            return false; 
        }
        item = _buffer[tail];
        _tail.store((tail + 1) % _capacity, std::memory_order_release);
        return true;
    }

private:
    const std::size_t _capacity;
    std::vector<T> _buffer;
    std::atomic<std::size_t> _head;
    std::atomic<std::size_t> _tail;
};

#endif // LOCK_FREE_QUEUE_HPP