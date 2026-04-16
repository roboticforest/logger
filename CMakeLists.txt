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
option(DVLOGGER_BUILD_DOCS "Build API documentation with Doxygen." ON)
option(DVLOGGER_ENABLE_MANUAL_TESTS "Enable manual/stress tests in CTest." OFF)

if(NOT DVLOGGER_BUILD_SHARED AND NOT DVLOGGER_BUILD_STATIC)
    message(FATAL_ERROR "At least one of DVLOGGER_BUILD_SHARED or DVLOGGER_BUILD_STATIC must be ON.")
endif()

set(DVLOGGER_GENERATED_DIR "${PROJECT_BINARY_DIR}/generated")
set(DVLOGGER_GENERATED_VERSION_HPP "${DVLOGGER_GENERATED_DIR}/version.hpp")
file(MAKE_DIRECTORY "${DVLOGGER_GENERATED_DIR}")
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/version.hpp.in"
    "${DVLOGGER_GENERATED_VERSION_HPP}"
    @ONLY
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

set(DVLOGGER_INSTALL_TARGETS)

function(dvlogger_attach_public_headers target_name)
    target_sources("${target_name}" PUBLIC
        FILE_SET dvlogger_public_headers
        TYPE HEADERS
        BASE_DIRS "${PROJECT_SOURCE_DIR}"
        FILES "${PROJECT_SOURCE_DIR}/logger.h"
    )
    target_sources("${target_name}" PUBLIC
        FILE_SET dvlogger_generated_headers
        TYPE HEADERS
        BASE_DIRS "${DVLOGGER_GENERATED_DIR}"
        FILES
            "${DVLOGGER_GENERATED_VERSION_HPP}"
    )
endfunction()

if(DVLOGGER_BUILD_SHARED)
    add_library(dvlogger_shared SHARED)
    dvlogger_apply_common_target_config(dvlogger_shared)
    dvlogger_attach_public_headers(dvlogger_shared)
    set_target_properties(dvlogger_shared PROPERTIES
        OUTPUT_NAME dvlogger
        EXPORT_NAME shared
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
    add_library(dvlogger::shared ALIAS dvlogger_shared)
    list(APPEND DVLOGGER_INSTALL_TARGETS dvlogger_shared)
endif()

if(DVLOGGER_BUILD_STATIC)
    add_library(dvlogger_static STATIC)
    dvlogger_apply_common_target_config(dvlogger_static)
    dvlogger_attach_public_headers(dvlogger_static)
    set_target_properties(dvlogger_static PROPERTIES
        OUTPUT_NAME dvlogger_static
        EXPORT_NAME static
    )
    add_library(dvlogger::static ALIAS dvlogger_static)
    list(APPEND DVLOGGER_INSTALL_TARGETS dvlogger_static)
endif()

####################################################################################################
# Documentation
####################################################################################################

if(DVLOGGER_BUILD_DOCS)
    find_package(Doxygen REQUIRED COMPONENTS doxygen)

    set(DOXYGEN_PROJECT_NAME "Logger")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DOXYGEN_PROJECT_BRIEF "A simple C++ logging utility library.")
    set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doc")
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_QUIET YES)
    set(DOXYGEN_RECURSIVE NO)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_WARNINGS NO)

    doxygen_add_docs(docs
        "${PROJECT_SOURCE_DIR}/logger.h"
        "${PROJECT_SOURCE_DIR}/logger.cpp"
        "${DVLOGGER_GENERATED_VERSION_HPP}"
        ALL
        COMMENT "Generating API documentation with Doxygen"
    )
endif()

####################################################################################################
# Testing
####################################################################################################

include(CTest)

if(BUILD_TESTING)
    if(TARGET dvlogger_static)
        set(DVLOGGER_TEST_LINK_TARGET dvlogger_static)
    else()
        set(DVLOGGER_TEST_LINK_TARGET dvlogger_shared)
    endif()

    function(dvlogger_add_test target_name source_file test_name test_labels)
        add_executable("${target_name}" "${source_file}")
        target_compile_features("${target_name}" PRIVATE cxx_std_20)
        target_link_libraries("${target_name}" PRIVATE "${DVLOGGER_TEST_LINK_TARGET}")
        add_test(NAME "${test_name}" COMMAND "${target_name}")
        set_tests_properties("${test_name}" PROPERTIES LABELS "${test_labels}")
    endfunction()

    dvlogger_add_test(
        dvlogger_smoke_test
        "${PROJECT_SOURCE_DIR}/tests/dvlogger_smoke_test.cpp"
        dvlogger.smoke
        "unit;smoke"
    )
    dvlogger_add_test(
        dvlogger_format_test
        "${PROJECT_SOURCE_DIR}/tests/logger_format_tests.cpp"
        dvlogger.format
        "unit;format"
    )
    dvlogger_add_test(
        dvlogger_stream_test
        "${PROJECT_SOURCE_DIR}/tests/logger_stream_tests.cpp"
        dvlogger.stream
        "unit;stream"
    )
    dvlogger_add_test(
        dvlogger_thread_test
        "${PROJECT_SOURCE_DIR}/tests/logger_thread_tests.cpp"
        dvlogger.thread
        "unit;thread"
    )
    dvlogger_add_test(
        dvlogger_failure_mode_test
        "${PROJECT_SOURCE_DIR}/tests/logger_failure_mode_tests.cpp"
        dvlogger.failure_mode
        "unit;failure_mode"
    )

    if(DVLOGGER_ENABLE_MANUAL_TESTS)
        dvlogger_add_test(
            dvlogger_stress_manual_test
            "${PROJECT_SOURCE_DIR}/tests/manual/dvlogger_stress_manual_test.cpp"
            dvlogger.manual.stress
            "manual;stress"
        )
        dvlogger_add_test(
            dvlogger_dead_stream_manual_test
            "${PROJECT_SOURCE_DIR}/tests/manual/dvlogger_dead_stream_manual_test.cpp"
            dvlogger.manual.dead_stream
            "manual;failure_mode"
        )
    endif()
endif()

####################################################################################################
# Install And Package
####################################################################################################

set(DVLOGGER_PACKAGE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/dvlogger")
set(DVLOGGER_PACKAGE_CONFIG_FILE "${PROJECT_BINARY_DIR}/dvlogger-config.cmake")
set(DVLOGGER_PACKAGE_VERSION_FILE "${PROJECT_BINARY_DIR}/dvlogger-config-version.cmake")
set(DVLOGGER_PACKAGE_TARGETS_FILE "dvloggerTargets.cmake")

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
    FILE_SET dvlogger_public_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILE_SET dvlogger_generated_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(EXPORT dvloggerTargets
    FILE "${DVLOGGER_PACKAGE_TARGETS_FILE}"
    NAMESPACE dvlogger::
    DESTINATION "${DVLOGGER_PACKAGE_CONFIG_INSTALL_DIR}"
)

install(FILES
    "${DVLOGGER_PACKAGE_CONFIG_FILE}"
    "${DVLOGGER_PACKAGE_VERSION_FILE}"
    DESTINATION "${DVLOGGER_PACKAGE_CONFIG_INSTALL_DIR}"
)
