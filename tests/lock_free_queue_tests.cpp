#include <gtest/gtest.h>
#include "../include/zerg/lock_free_queue.hpp"
#include <thread>
#include <vector>

class LockFreeQueueTest : public ::testing::Test
{
  protected:
    static constexpr size_t DEFAULT_CAPACITY = 16;
    LockFreeQueue<int> queue{DEFAULT_CAPACITY};
};

// basic Ops
TEST_F(LockFreeQueueTest, EnqueueDequeueBasic)
{
    EXPECT_TRUE(queue.enqueue(42));
    int value;
    EXPECT_TRUE(queue.dequeue(value));
    EXPECT_EQ(value, 42);
}

TEST_F(LockFreeQueueTest, EmptyQueueBehavior)
{
    int value;
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.dequeue(value));
}

TEST_F(LockFreeQueueTest, CapacityRounding)
{
    LockFreeQueue<int> q1{15};
    LockFreeQueue<int> q2{17};
    EXPECT_EQ(q1.capacity(), 16);
    EXPECT_EQ(q2.capacity(), 32);
}

TEST_F(LockFreeQueueTest, FullQueueBehavior)
{
    for (size_t i = 0; i < DEFAULT_CAPACITY - 1; ++i)
    {
        EXPECT_TRUE(queue.enqueue(i));
    }
    EXPECT_FALSE(queue.enqueue(42));
}

TEST_F(LockFreeQueueTest, ConcurrentEnqueueDequeue)
{
    static constexpr size_t NUM_OPERATIONS = 10000;
    std::atomic<bool> start{false};
    std::atomic<size_t> successful_enqueues{0};
    std::atomic<size_t> successful_dequeues{0};
    std::atomic<bool> producer_done{false};

    std::thread producer([&]() {
        while (!start)
        {
            std::this_thread::yield();
        }
        for (size_t i = 0; i < NUM_OPERATIONS; ++i)
        {
            if (queue.enqueue(i))
            {
                successful_enqueues++;
            }
        }
        producer_done = true;
    });

    std::thread consumer([&]() {
        while (!start)
        {
            std::this_thread::yield();
        }
        int value;
        while (!producer_done || !queue.isEmpty())
        {
            if (queue.dequeue(value))
            {
                successful_dequeues++;
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    start = true;
    producer.join();
    consumer.join();

    EXPECT_EQ(successful_enqueues.load(), successful_dequeues.load());
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(LockFreeQueueTest, StressTest)
{
    static constexpr size_t NUM_ITEMS = 100000;
    std::vector<int> produced(NUM_ITEMS);
    std::vector<int> consumed;
    consumed.reserve(NUM_ITEMS);

    std::thread producer([&]() {
        for (size_t i = 0; i < NUM_ITEMS; ++i)
        {
            produced[i] = i;
            while (!queue.enqueue(i))
            {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&]() {
        int value;
        size_t count = 0;
        while (count < NUM_ITEMS)
        {
            if (queue.dequeue(value))
            {
                consumed.push_back(value);
                count++;
            }
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(produced.size(), consumed.size());
    for (size_t i = 0; i < NUM_ITEMS; ++i)
    {
        EXPECT_EQ(produced[i], consumed[i]);
    }
}