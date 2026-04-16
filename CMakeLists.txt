####################################################################################################
# Project Setup
####################################################################################################

cmake_minimum_required(VERSION 4.0)
project(logger VERSION 0.77.4.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
find_package(Threads REQUIRED)

####################################################################################################
# Build Options
####################################################################################################

option(DVLOGGER_BUILD_SHARED "Build shared library target." ON)
option(DVLOGGER_BUILD_STATIC "Build static library target." ON)
option(DVLOGGER_WARNINGS_AS_ERRORS "Treat compiler warnings as errors (MSVC only)." OFF)
option(DVLOGGER_BUILD_DOCS "Build API documentation with Doxygen." ON)

if(NOT DVLOGGER_BUILD_SHARED AND NOT DVLOGGER_BUILD_STATIC)
    message(FATAL_ERROR "At least one of DVLOGGER_BUILD_SHARED or DVLOGGER_BUILD_STATIC must be ON.")
endif()

set(DVLOGGER_GENERATED_DIR "${PROJECT_BINARY_DIR}/generated")
set(DVLOGGER_GENERATED_VERSION_HPP "${DVLOGGER_GENERATED_DIR}/version.hpp")
set(DVLOGGER_GENERATED_VERSION_COMPAT_HEADER "${DVLOGGER_GENERATED_DIR}/logger_version.h")
file(MAKE_DIRECTORY "${DVLOGGER_GENERATED_DIR}")
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/version.hpp.in"
    "${DVLOGGER_GENERATED_VERSION_HPP}"
    @ONLY
)
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/logger_version.h.in"
    "${DVLOGGER_GENERATED_VERSION_COMPAT_HEADER}"
    @ONLY
)

set(DVLOGGER_PUBLIC_HEADERS
    "${PROJECT_SOURCE_DIR}/logger.h"
    "${DVLOGGER_GENERATED_VERSION_HPP}"
    "${DVLOGGER_GENERATED_VERSION_COMPAT_HEADER}"
)
set(DVLOGGER_SOURCES
    "${PROJECT_SOURCE_DIR}/logger.cpp"
)

####################################################################################################
# Target Definitions
####################################################################################################

function(dvlogger_apply_common_target_config target_name)
    target_sources("${target_name}" PRIVATE ${DVLOGGER_SOURCES})
    target_compile_features("${target_name}" PUBLIC cxx_std_20)
    target_include_directories("${target_name}"
        PUBLIC
            "$<BUILD_INTERFACE:${DVLOGGER_GENERATED_DIR}>"
            "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
    target_link_libraries("${target_name}" PUBLIC Threads::Threads)
endfunction()

function(dvlogger_apply_windows_warnings target_name)
    if(MSVC)
        target_compile_options("${target_name}" PRIVATE /W4 /permissive-)
        if(DVLOGGER_WARNINGS_AS_ERRORS)
            target_compile_options("${target_name}" PRIVATE /WX)
        endif()
    endif()
endfunction()

set(DVLOGGER_INSTALL_TARGETS)

if(DVLOGGER_BUILD_SHARED)
    add_library(dvlogger_shared SHARED)
    dvlogger_apply_common_target_config(dvlogger_shared)
    dvlogger_apply_windows_warnings(dvlogger_shared)
    set_target_properties(dvlogger_shared PROPERTIES
        OUTPUT_NAME dvlogger
        EXPORT_NAME shared
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
    list(APPEND DVLOGGER_INSTALL_TARGETS dvlogger_shared)
endif()

if(DVLOGGER_BUILD_STATIC)
    add_library(dvlogger_static STATIC)
    dvlogger_apply_common_target_config(dvlogger_static)
    dvlogger_apply_windows_warnings(dvlogger_static)
    set_target_properties(dvlogger_static PROPERTIES
        OUTPUT_NAME dvlogger_static
        EXPORT_NAME static
    )
    list(APPEND DVLOGGER_INSTALL_TARGETS dvlogger_static)
endif()

####################################################################################################
# Documentation
####################################################################################################

if(DVLOGGER_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    set(DVLOGGER_DOXYFILE_IN "${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in")
    set(DVLOGGER_DOXYFILE_OUT "${PROJECT_BINARY_DIR}/Doxyfile")
    set(DVLOGGER_DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/doc")
    configure_file("${DVLOGGER_DOXYFILE_IN}" "${DVLOGGER_DOXYFILE_OUT}" @ONLY)

    add_custom_target(docs ALL
        COMMAND "${DOXYGEN_EXECUTABLE}" "${DVLOGGER_DOXYFILE_OUT}"
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

####################################################################################################
# Testing
####################################################################################################

include(CTest)

if(BUILD_TESTING)
    add_executable(dvlogger_smoke_test "${PROJECT_SOURCE_DIR}/tests/dvlogger_smoke_test.cpp")
    target_compile_features(dvlogger_smoke_test PRIVATE cxx_std_20)

    if(TARGET dvlogger_static)
        target_link_libraries(dvlogger_smoke_test PRIVATE dvlogger_static)
    else()
        target_link_libraries(dvlogger_smoke_test PRIVATE dvlogger_shared)
    endif()

    add_test(NAME dvlogger.smoke COMMAND dvlogger_smoke_test)
endif()

####################################################################################################
# Install And Package
####################################################################################################

set(DVLOGGER_PACKAGE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/dvlogger")
set(DVLOGGER_PACKAGE_CONFIG_FILE "${PROJECT_BINARY_DIR}/dvlogger-config.cmake")
set(DVLOGGER_PACKAGE_VERSION_FILE "${PROJECT_BINARY_DIR}/dvlogger-config-version.cmake")

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/dvlogger-config.cmake.in"
    "${DVLOGGER_PACKAGE_CONFIG_FILE}"
    INSTALL_DESTINATION "${DVLOGGER_PACKAGE_CONFIG_INSTALL_DIR}"
)

write_basic_package_version_file(
    "${DVLOGGER_PACKAGE_VERSION_FILE}"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(TARGETS ${DVLOGGER_INSTALL_TARGETS}
    EXPORT dvloggerTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(FILES ${DVLOGGER_PUBLIC_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(EXPORT dvloggerTargets
    FILE dvlogger-targets.cmake
    NAMESPACE dvlogger::
    DESTINATION "${DVLOGGER_PACKAGE_CONFIG_INSTALL_DIR}"
)

install(FILES
    "${DVLOGGER_PACKAGE_CONFIG_FILE}"
    "${DVLOGGER_PACKAGE_VERSION_FILE}"
    DESTINATION "${DVLOGGER_PACKAGE_CONFIG_INSTALL_DIR}"
)
