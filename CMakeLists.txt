# Specify which version of CMake we're using.
cmake_minimum_required(VERSION 3.16)

# Set the names of each thing being built, the whole project, the library, and the library tester app.
set(PROJECT_NAME logger)
set(LIBRARY_NAME dvlogger)
set(PROGRAM_NAME logger_test)

# Version 1.0 checklist
### - Thread safe logging.
### - Tee'd output (logging individual messages to terminal and file at the same time).
### - Simple log file rotation.
project(${PROJECT_NAME} VERSION 0.7.0.0)
# configure_file(logger_version.clion.template logger_version.h)
# The following code gives more control and flexibility than the above line/setup.
file(WRITE ${PROJECT_NAME}_version.h
        "// GENERATED FILE - Adjust content via CMakeLists.txt for this project.\n"
        "#define LOGGER_VERSION_MAJOR " ${${PROJECT_NAME}_VERSION_MAJOR} "\n"
        "#define LOGGER_VERSION_MINOR " ${${PROJECT_NAME}_VERSION_MINOR} "\n"
        "#define LOGGER_VERSION_PATCH " ${${PROJECT_NAME}_VERSION_PATCH} "\n"
        "#define LOGGER_VERSION_TWEAK " ${${PROJECT_NAME}_VERSION_TWEAK} "\n"
        )

# Choose the C++ language standard the compiler should use.
set(CMAKE_CXX_STANDARD 17)

# Build the logging library, setting compiler specific warning flags where appropriate.
add_library(${LIBRARY_NAME} STATIC
        logger.cpp
        )

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( ${LIBRARY_NAME} PRIVATE -Wall -Wextra -Wunreachable-code -Wpedantic)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( ${LIBRARY_NAME} PRIVATE /WX /W4 )
endif()

# Build the test program executable.
add_executable(${PROGRAM_NAME} main.cpp)
target_link_libraries(${PROGRAM_NAME} PUBLIC ${LIBRARY_NAME})
