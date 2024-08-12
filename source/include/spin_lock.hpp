#pragma once
#include <atomic>
#include <thread>

class SpinLock {
public:
    void acquire() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // Spin until the lock is acquired
            std::this_thread::yield();
        }
    }

    void release() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag {};
};

class Guard{
public:
    Guard(SpinLock& lock) : spin_lock(lock) 
    {
        spin_lock.acquire();
    }
    ~Guard()
    {
        spin_lock.release();
    }
private:
    SpinLock &spin_lock;
};