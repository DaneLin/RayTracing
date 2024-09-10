#include "thread/thread_pool.hpp"
#include <utils/profile.hpp>
#include <cmath>

ThreadPool thread_pool {};
void ThreadPool::WorkerThread(ThreadPool *master)
{
    while (master->alive == 1)
    {
        if (master->tasks.empty())
        {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
        }
        Task* task = master->getTask();
        if (task != nullptr)
        {
            task->run();
            delete task;
            master->pending_task_count--;
        }
        else 
        {
            std::this_thread::yield();
        }
    }
}

ThreadPool::ThreadPool(size_t thread_count)
{
    alive = 1;
    pending_task_count = 0;
    if (thread_count == 0)
    {
        thread_count = std::thread::hardware_concurrency();
        //thread_count = 1;
    }
    for (size_t i =0 ; i <thread_count;i++)
    {
        threads.push_back(std::thread(ThreadPool::WorkerThread, this));
    }
}

ThreadPool::~ThreadPool()
{
    wait();
    alive = 0;
    for (auto &thread:threads)
    {
        thread.join();
    }
    threads.clear();
}

void ThreadPool::addTask(Task *task)
{
    Guard guard(spin_lock);
    tasks.push_back(task);
    pending_task_count++;
}

class ParallelForTask : public Task
{
private:

    size_t x, y, chunk_width, chunk_height;
    std::function<void(size_t, size_t)> lambda;
public:
    ParallelForTask(size_t x, size_t y, size_t chunk_width, size_t chunk_height, const std::function<void(size_t, size_t)> &lambda)
        :x(x), y(y),chunk_width(chunk_width), chunk_height(chunk_height), lambda(lambda){}

    void run() override
    {
        for (size_t idx_x  = 0; idx_x < chunk_width; idx_x++)
        {
            for (size_t idx_y = 0; idx_y < chunk_height; idx_y++)
            {
                lambda(x + idx_x, y+idx_y);
            }
        }
    }
};

void ThreadPool::parallelFor(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda, bool bIsComplex)
{
    PROFILE("ParallelFor")
    Guard guard(spin_lock);

    float chunk_width_float = static_cast<float>(width) / sqrt(threads.size());
    float chunk_height_float = static_cast<float>(height)  / sqrt(threads.size());

    if (bIsComplex)
    {
        chunk_height_float /= sqrt(16);
        chunk_width_float /= sqrt(16);
    }

    size_t chunk_width = std::ceil(chunk_width_float);
    size_t chunk_height = std::ceil(chunk_height_float);

    for (size_t x = 0; x < width; x+= chunk_width)
    {
        for (size_t y = 0; y < height; y+=chunk_height)
        {
            size_t current_chunk_width = chunk_width;
            size_t current_chunk_height = chunk_height;
            if (x + chunk_width > width)
            {
                current_chunk_width = width - x;
            }
            if (y + chunk_height > height)
            {
                current_chunk_height = height - y;
            }
            tasks.push_back(new ParallelForTask(x, y,current_chunk_width, current_chunk_height, lambda));
            pending_task_count++;
        }
    }
}

void ThreadPool::wait() const
{
    while(pending_task_count > 0)
    {
        std::this_thread::yield();
    }
}

Task *ThreadPool::getTask()
{
    Guard guard(spin_lock);
    if (tasks.empty())
    {
        return nullptr;
    }

    Task* task = tasks.front();
    tasks.pop_front();
    return task;
}
