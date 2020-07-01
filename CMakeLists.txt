# Specify which version of CMake we're using.
cmake_minimum_required(VERSION 3.16)

# Set the project name.
project(logger)

# Choose the C++ language standard the compiler should use.
set(CMAKE_CXX_STANDARD 17)

# Build the logging library.
add_library(dvlogger STATIC logger.cpp)

# Build the executable.
add_executable(logger_test main.cpp)
target_link_libraries(logger_test PUBLIC dvlogger)
