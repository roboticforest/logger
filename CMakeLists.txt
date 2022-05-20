####################################################################################################
# All rights reserved © 2020 David Vitez (AKA: Robotic Forest)
# This software is the private intellectual property of David Vitez unless otherwise stated.
####################################################################################################

####################################################################################################
# General CMake project config.
####################################################################################################

# Specify the version being used and include some custom utility functions.
cmake_minimum_required(VERSION 3.16)
# The following CMake files are a separate project and their exact path depends on programmer preference.
set(CUSTOM_CMAKE_FN_PATH "../cmake")
include("${CUSTOM_CMAKE_FN_PATH}/make_cpp_version_header.cmake")
include("${CUSTOM_CMAKE_FN_PATH}/make_cmake_config_file.cmake")
include("${CUSTOM_CMAKE_FN_PATH}/make_cmake_config_version_file.cmake")

# Set the names of each thing being built, this library project, the library itself, and the library tester app.
set(PROJECT_NAME logger)
set(LIBRARY_NAME dvlogger)
set(PROGRAM_NAME logger_test)

# Library Version 1.0 checklist
# Each line is +(1/9) to the minor version.
# Each "Bug" will add +1 to the patch (until next ++ of minor).
# Each "Tweak" will add +1 to the tweak (until next ++ of minor).
### X Print to any supplied output stream (console or file).
### X Print the name of the logger.
### X Print the logging level.
### X Print in color.
### X Bug - Print in color only when printing to cout so files aren't filled with garbage color codes.
### X Print a timestamp as precise as possible as YYYY-MM-DD HH:MM:SS:something (ended up being nanoseconds).
### X Tweak - print out timezone.
### - Option to append to log files. (Currently it always overwrites the entire file.)
### X Thread safe logging. (Streams are already safe, but output from many can mix.)
### X Tee'd output (logging individual messages to terminal and file at the same time).
### - Simple log file rotation. (File size limit.)
set(MAJOR 0)
set(MINOR 77) # 7/9 Splitting output.
set(PATCH 3)  # Compilation fixes, and nanosecond format bug.
set(TWEAK 0)
project(${PROJECT_NAME} VERSION ${MAJOR}.${MINOR}.${PATCH}.${TWEAK})

####################################################################################################
# Source code and compilation.
####################################################################################################

# Choose the C++ language standard the compiler should use.
set(CMAKE_CXX_STANDARD 17)
# Ensure threading works.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Generate a header that will make the code aware of its own version.
# Generates "logger_version.h"
make_cpp_version_header(
    NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    PATH "."
)

# Build the logging library, setting compiler specific warning ond compilation flags where appropriate.
set(SRC_FILES_LIB logger.cpp)
set(SRC_FILES_TEST main.cpp)
set(INC_FILES logger.h logger_version.h)
# message("${SRC_FILES}") # For debugging this CMake file.
add_library(${LIBRARY_NAME} STATIC "${SRC_FILES_LIB}")
if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    # -fPIC makes "Position Independent Code" which will allow this library to be torn apart and integrated into
    # other dynamic shared libraries and not just usable by executable files alone.
    target_compile_options( ${LIBRARY_NAME} PRIVATE -Wall -Wextra -Wunreachable-code -Wpedantic -fPIC)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( ${LIBRARY_NAME} PRIVATE /WX /W4 )
endif()

# Build the test program executable.
add_executable(${PROGRAM_NAME} "${SRC_FILES_TEST}")
target_link_libraries(${PROGRAM_NAME} PRIVATE ${LIBRARY_NAME} Threads::Threads)

####################################################################################################
# Packaging and export.
####################################################################################################

# Create library configuration files for other projects to use.
# This generates both a <library_name>-config.cmake and a <library_name>-config-version.cmake.
make_cmake_config_file(
    NAME ${LIBRARY_NAME}
    VAR_PREFIX ${PROJECT_NAME}
    LIB_NAME ${LIBRARY_NAME}
)
make_cmake_config_version_file(
    NAME ${LIBRARY_NAME}
    VERSION ${PROJECT_VERSION}
)

# Prep for later packaging.
# You must manually run `cmake --install <build-folder>` from the project root directory.
# The project root directory should be the folder this file is contained in.
set(PACKAGE_DEST "${PROJECT_SOURCE_DIR}/${LIBRARY_NAME}-${PROJECT_VERSION}-${CMAKE_BUILD_TYPE}")
install(TARGETS ${LIBRARY_NAME} DESTINATION "${PACKAGE_DEST}/lib")
install(TARGETS ${PROGRAM_NAME} DESTINATION "${PACKAGE_DEST}/bin")
install(FILES ${INC_FILES} DESTINATION "${PACKAGE_DEST}/include")
install(FILES "${PROJECT_BINARY_DIR}/${LIBRARY_NAME}-config.cmake" DESTINATION "${PACKAGE_DEST}/cmake")
install(FILES "${PROJECT_BINARY_DIR}/${LIBRARY_NAME}-config-version.cmake" DESTINATION "${PACKAGE_DEST}/cmake")

