#include "../include/cpp_logger/lock_free_queue.hpp"
#include <pthread.h>
#include <vector>
#include <chrono>

static void *producer_thread(void *arg)
{
    auto *queue = static_cast<LockFreeQueue<int> *>(arg);
    for (int i = 0; i < 100; ++i)
    {
        while (!queue->enqueue(i))
        {
            __asm__ volatile("pause" ::: "memory");
        }
    }
    return nullptr;
}

static void *consumer_thread(void *arg)
{
    auto *queue = static_cast<LockFreeQueue<int> *>(arg);
    int value;
    int count = 0;

    while (count < 100)
    {
        if (queue->dequeue(value))
        {
            count++;
        }
        else
        {
            __asm__ volatile("pause" ::: "memory");
        }
    }
    return nullptr;
}

int main()
{
    LockFreeQueue<int> queue(1024);

    constexpr int NUM_THREADS = 4;
    std::vector<pthread_t> producers(NUM_THREADS);
    std::vector<pthread_t> consumers(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_create(&producers[i], nullptr, producer_thread, &queue);
        pthread_create(&consumers[i], nullptr, consumer_thread, &queue);
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(producers[i], nullptr);
        pthread_join(consumers[i], nullptr);
    }

    return 0;
}