#include "../include/cpp_logger/lock_free_queue.hpp"
#include <pthread.h>

static void* producer_thread(void* arg) {
    auto* queue = static_cast<LockFreeQueue<int>*>(arg);
    bool success = true;
    for (int i = 0; i < 1000 && success; ++i) {
        success = queue->enqueue(i);
    }
    return reinterpret_cast<void*>(static_cast<uintptr_t>(success));
}

static bool test_queue() {
    LockFreeQueue<int> queue(1024);
    pthread_t thread;
    if (pthread_create(&thread, nullptr, producer_thread, &queue) != 0) {
        return false;
    }
    
    int value;
    bool success = true;
    for (int i = 0; i < 1000 && success; ++i) {
        while (!queue.dequeue(value)) {
            __asm__ volatile("pause" ::: "memory");
        }
        success = (value == i);
    }
    
    void* thread_result;
    pthread_join(thread, &thread_result);
    return success && static_cast<bool>(reinterpret_cast<uintptr_t>(thread_result));
}

int main() {
    return test_queue() ? 0 : 1;
}

template class LockFreeQueue<int>;