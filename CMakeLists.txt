# Specify which version of CMake we're using.
cmake_minimum_required(VERSION 3.16)

# Choose the C++ language standard the compiler should use.
set(CMAKE_CXX_STANDARD 17)

# Set the project name.
project(dv_logger_test)

# Assemble executable.
add_executable(dv_logger_test main.cpp logger.cpp)
