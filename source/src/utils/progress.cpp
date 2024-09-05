#include "utils/progress.hpp"

#include <iostream>



Progress::Progress(size_t total, size_t step)
    :total(total), current(0), step(step), last_percent(0),percent(0)
{
    std::cout << "\rCurrent Progress: 0%" << std::endl;
}
void Progress::update(size_t count)
{
    Guard guard(lock);

    current += count;
    percent = 100 * static_cast<double>(current) / static_cast<double>(total);
    if (percent - last_percent >= step)
    {
        last_percent = percent;
        std::cout << "\rCurrent Progress: " << percent << "%" << std::endl;
    }
    if (percent == 100)
    {
        std::cout << "\nFinished!" << std::endl;
    }
}