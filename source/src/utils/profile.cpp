#include "utils/profile.hpp"
#include <iostream>

Profile::Profile(const std::string& name)
    :name(name), start(std::chrono::high_resolution_clock::now())
{

}

Profile::~Profile()
{
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout <<  "Profile \"" << name << "\": " << duration << "ms" << std::endl;
}
