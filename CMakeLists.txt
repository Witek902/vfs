cmake_minimum_required(VERSION 2.6)
project(vfs)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wpedantic -O0 -g")
add_executable(vfs test.cpp vfs.cpp)
