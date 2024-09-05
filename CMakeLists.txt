set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.5.0)
project(CPURayTracing VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(thirdparty/rapidobj)
add_subdirectory(thirdparty/glm)
add_subdirectory(source)

