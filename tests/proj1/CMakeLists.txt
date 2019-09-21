cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(test_proj)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

add_executable(main "src/main.cpp")
target_include_directories(main PRIVATE "include")
