#pragma once

#include "thread/spin_lock.hpp"

class Progress
{
public:
    Progress(size_t total, size_t step = 10);

    void update(size_t count);

private:
    size_t total, current;
    size_t percent, last_percent, step;
    SpinLock lock;
};