# All rights reserved © 2020-2026 David Vitez
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

####################################################################################################
# Project Setup
####################################################################################################

cmake_minimum_required(VERSION 4.0)
project(logger VERSION 0.90.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
find_package(Threads REQUIRED)

####################################################################################################
# Build Options
####################################################################################################

option(DVLOGGER_BUILD_SHARED "Build shared library target." ON)
option(DVLOGGER_BUILD_STATIC "Build static library target." ON)
option(DVLOGGER_BUILD_PUBLIC_DOCS "Build API documentation with Doxygen." ON)
option(DVLOGGER_BUILD_INTERNAL_DOCS "Build private developer documentation with Doxygen." ON)
option(DVLOGGER_ENABLE_MANUAL_TESTS "Enable manual/stress tests in CTest." OFF)

if(NOT DVLOGGER_BUILD_SHARED AND NOT DVLOGGER_BUILD_STATIC)
    message(FATAL_ERROR "At least one of DVLOGGER_BUILD_SHARED or DVLOGGER_BUILD_STATIC must be ON.")
endif()

####################################################################################################
# Target Definitions
####################################################################################################

set(DVLOGGER_INSTALL_TARGETS)

function(dvlogger_add_target target_name library_type output_name export_name)
    add_library("${target_name}" "${library_type}")
    target_compile_features("${target_name}" PUBLIC cxx_std_20)
    target_sources("${target_name}"
        PRIVATE
            "${PROJECT_SOURCE_DIR}/logger.cpp"
        PUBLIC
            FILE_SET dvlogger_modules
            TYPE CXX_MODULES
            BASE_DIRS "${PROJECT_SOURCE_DIR}"
            FILES "${PROJECT_SOURCE_DIR}/logger.cppm"
    )
    target_compile_definitions("${target_name}" PUBLIC
        DVLOGGER_VERSION_STRING="${PROJECT_VERSION}"
        DVLOGGER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        DVLOGGER_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        DVLOGGER_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        DVLOGGER_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
    )
    target_link_libraries("${target_name}" PUBLIC Threads::Threads)
    set_target_properties("${target_name}" PROPERTIES
        OUTPUT_NAME "${output_name}"
        EXPORT_NAME "${export_name}"
    )
endfunction()

if(DVLOGGER_BUILD_SHARED)
    dvlogger_add_target(dvlogger_shared SHARED dvlogger shared)
    set_target_properties(dvlogger_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_library(dvlogger::shared ALIAS dvlogger_shared)
    list(APPEND DVLOGGER_INSTALL_TARGETS dvlogger_shared)
endif()

if(DVLOGGER_BUILD_STATIC)
    dvlogger_add_target(dvlogger_static STATIC dvlogger_static static)
    add_library(dvlogger::static ALIAS dvlogger_static)
    list(APPEND DVLOGGER_INSTALL_TARGETS dvlogger_static)
endif()

####################################################################################################
# Documentation
####################################################################################################

if(DVLOGGER_BUILD_PUBLIC_DOCS)
    find_package(Doxygen REQUIRED COMPONENTS doxygen)
    set(DVLOGGER_DOC_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DVLOGGER_DOC_EXCLUDE_PATTERNS "*/.git/* */.svn/* */.hg/* */CMakeFiles/* */_CPack_Packages/* DartConfiguration.tcl CMakeLists.txt CMakeCache.txt")
    string(TIMESTAMP DVLOGGER_DOC_COPYRIGHT_YEAR "%Y")
    set(DVLOGGER_DOC_COPYRIGHT_TEXT "Copyright © ${DVLOGGER_DOC_COPYRIGHT_YEAR} David Vitez & DV::Logger contributors.")
    set(DVLOGGER_DOC_LICENSE_TEXT "Licensed under Mozilla Public License, v. 2.0. <a href=\"https://mozilla.org/MPL/2.0/\">mozilla.org</a>")
    set(DVLOGGER_DOC_FOOTER_INPUT "${PROJECT_SOURCE_DIR}/cmake/DoxygenFooter.html.in")
    set(DVLOGGER_DOC_FOOTER_OUTPUT "${PROJECT_BINARY_DIR}/DoxygenFooter.html")
    set(DVLOGGER_DOC_PUBLIC_INPUT_FILES
        "${PROJECT_SOURCE_DIR}/logger.cppm"
        "${PROJECT_SOURCE_DIR}/docs/basic-usage.md"
        "${PROJECT_SOURCE_DIR}/docs/log-levels.md"
        "${PROJECT_SOURCE_DIR}/docs/api-reference.md"
    )
    set(DVLOGGER_DOC_INTERNAL_INPUT_FILES
        ${DVLOGGER_DOC_PUBLIC_INPUT_FILES}
        "${PROJECT_SOURCE_DIR}/logger.cpp"
    )
    set(DVLOGGER_DOC_PUBLIC_INPUTS ${DVLOGGER_DOC_PUBLIC_INPUT_FILES})
    list(TRANSFORM DVLOGGER_DOC_PUBLIC_INPUTS PREPEND "\"")
    list(TRANSFORM DVLOGGER_DOC_PUBLIC_INPUTS APPEND "\"")
    string(JOIN " " DVLOGGER_DOC_PUBLIC_INPUTS ${DVLOGGER_DOC_PUBLIC_INPUTS})
    set(DVLOGGER_DOC_PUBLIC_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doc/public")
    set(DVLOGGER_DOC_PUBLIC_PROJECT_BRIEF "Public API reference for the Logger library.")
    set(DVLOGGER_DOC_PUBLIC_EXCLUDE_SYMBOLS "DV::Logger::Impl")
    set(DVLOGGER_DOC_PUBLIC_HTML_FOOTER "${DVLOGGER_DOC_FOOTER_OUTPUT}")
    set(DVLOGGER_DOC_PUBLIC_CONFIG_INPUT "${PROJECT_SOURCE_DIR}/cmake/Doxyfile.public.in")
    set(DVLOGGER_DOC_PUBLIC_CONFIG_OUTPUT "${PROJECT_BINARY_DIR}/Doxyfile.docs_public")

    configure_file(
        "${DVLOGGER_DOC_FOOTER_INPUT}"
        "${DVLOGGER_DOC_FOOTER_OUTPUT}"
        @ONLY
    )

    configure_file(
        "${DVLOGGER_DOC_PUBLIC_CONFIG_INPUT}"
        "${DVLOGGER_DOC_PUBLIC_CONFIG_OUTPUT}"
        @ONLY
    )

    add_custom_target(docs_public ALL
        COMMAND "${DOXYGEN_EXECUTABLE}" "${DVLOGGER_DOC_PUBLIC_CONFIG_OUTPUT}"
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        DEPENDS
            "${DVLOGGER_DOC_PUBLIC_CONFIG_INPUT}"
            ${DVLOGGER_DOC_PUBLIC_INPUT_FILES}
        COMMENT "Generating docs_public documentation with Doxygen"
        VERBATIM
    )

    if(DVLOGGER_BUILD_INTERNAL_DOCS)
        set(DVLOGGER_DOC_INTERNAL_INPUTS ${DVLOGGER_DOC_INTERNAL_INPUT_FILES})
        list(TRANSFORM DVLOGGER_DOC_INTERNAL_INPUTS PREPEND "\"")
        list(TRANSFORM DVLOGGER_DOC_INTERNAL_INPUTS APPEND "\"")
        string(JOIN " " DVLOGGER_DOC_INTERNAL_INPUTS ${DVLOGGER_DOC_INTERNAL_INPUTS})
        set(DVLOGGER_DOC_INTERNAL_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doc/internal")
        set(DVLOGGER_DOC_INTERNAL_PROJECT_BRIEF "Private developer reference for the Logger library internals.")
        set(DVLOGGER_DOC_INTERNAL_HTML_FOOTER "${DVLOGGER_DOC_FOOTER_OUTPUT}")
        set(DVLOGGER_DOC_INTERNAL_CONFIG_INPUT "${PROJECT_SOURCE_DIR}/cmake/Doxyfile.internal.in")
        set(DVLOGGER_DOC_INTERNAL_CONFIG_OUTPUT "${PROJECT_BINARY_DIR}/Doxyfile.docs_internal")

        configure_file(
            "${DVLOGGER_DOC_INTERNAL_CONFIG_INPUT}"
            "${DVLOGGER_DOC_INTERNAL_CONFIG_OUTPUT}"
            @ONLY
        )

        add_custom_target(docs_internal
            COMMAND "${DOXYGEN_EXECUTABLE}" "${DVLOGGER_DOC_INTERNAL_CONFIG_OUTPUT}"
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            DEPENDS
                "${DVLOGGER_DOC_INTERNAL_CONFIG_INPUT}"
                ${DVLOGGER_DOC_INTERNAL_INPUT_FILES}
            COMMENT "Generating docs_internal documentation with Doxygen"
            VERBATIM
        )
    endif()

    add_custom_target(docs_all)
    add_dependencies(docs_all docs_public)
    if(TARGET docs_internal)
        add_dependencies(docs_all docs_internal)
    endif()
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
    FILE_SET dvlogger_modules DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/modules"
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
