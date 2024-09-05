#pragma once

#include <chrono>
#include <string>

#define PROFILE(name) Profile __profile(name);

class Profile
{
public:
    Profile(const std::string& name);

    ~Profile();

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start;
};