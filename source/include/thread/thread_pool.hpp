#pragma once

#include <vector>
#include <thread>
#include <list>
#include <functional>
#include "spin_lock.hpp"

class Task
{
public:
    virtual void run() = 0;
};

class ThreadPool
{
public:
    static void WorkerThread(ThreadPool *master);

    ThreadPool(size_t thread_count = 0);
    ~ThreadPool();

    void parallelFor(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda);
    void wait() const;

    void addTask(Task *task);
    Task *getTask();

private:
    std::atomic<int> alive;
    std::vector<std::thread> threads;
    std::atomic<int> pending_task_count;
    std::list<Task *> tasks;
    SpinLock spin_lock{};
};

extern ThreadPool thread_pool;