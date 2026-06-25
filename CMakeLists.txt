# Copyright © 2020-2026 David Vitez
# All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

########################################################################
# Project Setup
########################################################################

cmake_minimum_required(VERSION 4.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(DVLogger
        VERSION 0.91.0.0
        DESCRIPTION "A simple C++ logging utility library."
        HOMEPAGE_URL "https://github.com/roboticforest/logger"
        LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

find_package(Threads REQUIRED)

option(ENABLE_DOCS_BUILDING "Enable building of Doxygen documentation." ON)
option(ENABLE_MANUAL_TESTS_BUILDING "Enable building and running manual/stress tests in CTest." ON)
option(BUILD_INSTALLATION_FILES "Create CMake config files for consumers." ON)

set(SRC_FILES "${PROJECT_SOURCE_DIR}/logger.cpp")
set(SRC_MODULE_FILES "${PROJECT_SOURCE_DIR}/logger.cppm")

########################################################################
# Library Targets
########################################################################

add_library(dvlogger_shared SHARED)
target_compile_features(dvlogger_shared PUBLIC cxx_std_20)
target_sources(dvlogger_shared
        PRIVATE
        ${SRC_FILES}
        PUBLIC
        FILE_SET src_mod_file_set
        TYPE CXX_MODULES
        BASE_DIRS "${PROJECT_SOURCE_DIR}"
        FILES ${SRC_MODULE_FILES}
)
target_compile_definitions(dvlogger_shared
        PUBLIC
        DVLOGGER_VERSION_STRING="${PROJECT_VERSION}"
        DVLOGGER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        DVLOGGER_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        DVLOGGER_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        DVLOGGER_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)
target_link_libraries(dvlogger_shared PUBLIC Threads::Threads)
set_target_properties(dvlogger_shared PROPERTIES
        OUTPUT_NAME "dvlogger"
        EXPORT_NAME "Logger"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)
add_library(DV::Logger ALIAS dvlogger_shared)

add_library(dvlogger_static STATIC)
target_compile_features(dvlogger_static PUBLIC cxx_std_20)
target_sources(dvlogger_static
        PRIVATE
        ${SRC_FILES}
        PUBLIC
        FILE_SET src_mod_file_set
        TYPE CXX_MODULES
        BASE_DIRS "${PROJECT_SOURCE_DIR}"
        FILES ${SRC_MODULE_FILES}
)
target_compile_definitions(dvlogger_static
        PUBLIC
        DVLOGGER_VERSION_STRING="${PROJECT_VERSION}"
        DVLOGGER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        DVLOGGER_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        DVLOGGER_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        DVLOGGER_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)
target_link_libraries(dvlogger_static PUBLIC Threads::Threads)
set_target_properties(dvlogger_static PROPERTIES
        OUTPUT_NAME "dvlogger_static"
        EXPORT_NAME "LoggerStatic"
)
add_library(DV::LoggerStatic ALIAS dvlogger_static)

########################################################################
# Install/Export
########################################################################

if (BUILD_INSTALLATION_FILES)
    set(package_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
    set(package_config_file "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
    set(package_version_file "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")

    configure_package_config_file(
            "${PROJECT_SOURCE_DIR}/cmake/ProjectConfig.cmake.in"
            "${package_config_file}"
            INSTALL_DESTINATION "${package_dir}"
    )

    write_basic_package_version_file(
            "${package_version_file}"
            VERSION "${PROJECT_VERSION}"
            COMPATIBILITY SameMajorVersion
    )

    install(TARGETS
            dvlogger_shared
            dvlogger_static
            EXPORT DVLoggerTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            FILE_SET src_mod_file_set DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/modules"
    )

    install(EXPORT DVLoggerTargets
            FILE "${PROJECT_NAME}Targets.cmake"
            NAMESPACE DV::
            DESTINATION "${package_dir}"
    )

    install(FILES
            "${package_config_file}"
            "${package_version_file}"
            DESTINATION "${package_dir}"
    )
endif ()

########################################################################
# Tests
########################################################################

if (BUILD_TESTING)
    include("${PROJECT_SOURCE_DIR}/cmake/add-cpp-test.cmake")

    set(UNIT_TESTS_LIST
            simple_smoke_test
            basic_test
            stream_split_test
            threading_test
            failure_mode_test
    )
    set(MANUAL_TESTS_LIST
            manual_stress_test
            manual_deleted_stream_test
    )

    add_cpp_test(
            simple_smoke_test
            "${PROJECT_SOURCE_DIR}/tests/logging-smoke-test.cpp"
            "unit;smoke"
            dvlogger_static
    )
    add_cpp_test(
            basic_test
            "${PROJECT_SOURCE_DIR}/tests/basic-test.cpp"
            "unit;format"
            dvlogger_static
    )
    add_cpp_test(
            stream_split_test
            "${PROJECT_SOURCE_DIR}/tests/stream-tests.cpp"
            "unit;stream"
            dvlogger_static
    )
    add_cpp_test(
            threading_test
            "${PROJECT_SOURCE_DIR}/tests/simple-threading-tests.cpp"
            "unit;thread"
            dvlogger_static
    )
    add_cpp_test(
            failure_mode_test
            "${PROJECT_SOURCE_DIR}/tests/failure-mode-tests.cpp"
            "unit;failure_mode"
            dvlogger_static
    )

    if (ENABLE_MANUAL_TESTS_BUILDING)
        add_cpp_test(
                manual_stress_test
                "${PROJECT_SOURCE_DIR}/tests/manual/threading-stress-manual-test.cpp"
                "manual;stress"
                dvlogger_static
        )
        add_cpp_test(
                manual_deleted_stream_test
                "${PROJECT_SOURCE_DIR}/tests/manual/dead-stream-manual-test.cpp"
                "manual;failure_mode"
                dvlogger_static
        )
    endif ()

    add_custom_target(base_unit_tests
            COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C "$<CONFIG>" -L unit
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            USES_TERMINAL
            DEPENDS ${UNIT_TESTS_LIST}
    )
    set(ALL_ACTIVE_TESTS_LIST ${UNIT_TESTS_LIST})

    if (BUILD_INSTALLATION_FILES)
        add_test(
                NAME installation_smoke_test
                COMMAND
                "${CMAKE_COMMAND}"
                "-Ddvlogger_binary_dir=${PROJECT_BINARY_DIR}"
                "-Ddvlogger_consumer_source_dir=${PROJECT_SOURCE_DIR}/tests/install"
                "-Ddvlogger_install_prefix=${PROJECT_BINARY_DIR}/install-smoke/prefix"
                "-Ddvlogger_consumer_binary_dir=${PROJECT_BINARY_DIR}/install-smoke/build"
                "-Ddvlogger_build_config=$<CONFIG>"
                "-Ddvlogger_generator=${CMAKE_GENERATOR}"
                "-P"
                "${PROJECT_SOURCE_DIR}/tests/install/installation-smoke-test.cmake"
        )
        set_tests_properties(installation_smoke_test PROPERTIES LABELS "integration;install")
        list(APPEND ALL_ACTIVE_TESTS_LIST dvlogger_shared dvlogger_static)
    endif ()

    if (ENABLE_MANUAL_TESTS_BUILDING)
        list(APPEND ALL_ACTIVE_TESTS_LIST ${MANUAL_TESTS_LIST})
        add_custom_target(manual_unit_tests
                COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C "$<CONFIG>" -L manual
                WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                USES_TERMINAL
                DEPENDS ${MANUAL_TESTS_LIST}
        )
    else ()
        add_custom_target(manual_unit_tests
                COMMAND "${CMAKE_COMMAND}" -E echo "Manual tests are disabled."
                USES_TERMINAL
        )
    endif ()
endif ()

add_custom_target(all_active_unit_tests
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -C "$<CONFIG>"
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        USES_TERMINAL
        DEPENDS ${ALL_ACTIVE_TESTS_LIST}
)

########################################################################
# Docs
########################################################################

if (ENABLE_DOCS_BUILDING)
    find_package(Doxygen REQUIRED COMPONENTS doxygen)
    include("${PROJECT_SOURCE_DIR}/cmake/list-to-quoted-arg-str.cmake")

    # Common Docs and Information
    string(TIMESTAMP DOXY_COPYRIGHT_YEAR "%Y")
    set(DOXY_SHARED_OPTIONS_TEMPLATE_FILE "${PROJECT_SOURCE_DIR}/cmake/Doxyfile.common.in")
    set(DOXY_SHARED_OPTIONS_GEN_OUTPUT_FILE "${PROJECT_BINARY_DIR}/Doxyfile.common")
    set(DOXY_FOOTER_CONFIG_TEMPLATE_FILE "${PROJECT_SOURCE_DIR}/cmake/DoxygenFooter.html.in")
    set(DOXY_FOOTER_GEN_OUTPUT_FILE "${PROJECT_BINARY_DIR}/DoxygenFooter.html")
    set(DOXY_COMMON_MAIN_PAGE "${PROJECT_SOURCE_DIR}/docs/basic-usage.md")

    # Public API Docs
    set(DOXY_PUBLIC_INPUT_FILES
            "${PROJECT_SOURCE_DIR}/logger.cppm"
            "${PROJECT_SOURCE_DIR}/docs"
    )
    list_to_quoted_arg_str(DOXY_PUBLIC_INPUT_FILES DOXY_PUBLIC_INPUTS)

    set(DOXY_PUBLIC_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doc/public")
    set(DOXY_PUBLIC_CONFIG_TEMPLATE_FILE "${PROJECT_SOURCE_DIR}/cmake/Doxyfile.public.in")
    set(DOXY_PUBLIC_CONFIG_GEN_OUTPUT_FILE "${PROJECT_BINARY_DIR}/Doxyfile.docs_public")
    set(DOXY_PUBLIC_EXCLUDE_SYMBOLS "DV::Logger::Impl")

    # Internal Developer Docs
    set(DOXY_INTERNAL_INPUT_FILES
            ${DOXY_PUBLIC_INPUT_FILES}
            "${PROJECT_SOURCE_DIR}/logger.cpp"
    )
    list_to_quoted_arg_str(DOXY_INTERNAL_INPUT_FILES DOXY_INTERNAL_INPUTS)
    set(DOXY_INTERNAL_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doc/internal")
    set(DOXY_INTERNAL_CONFIG_TEMPLATE_FILE "${PROJECT_SOURCE_DIR}/cmake/Doxyfile.internal.in")
    set(DOXY_INTERNAL_CONFIG_GEN_OUTPUT_FILE "${PROJECT_BINARY_DIR}/Doxyfile.docs_internal")

    # Generated files.
    configure_file("${DOXY_SHARED_OPTIONS_TEMPLATE_FILE}" "${DOXY_SHARED_OPTIONS_GEN_OUTPUT_FILE}" @ONLY)
    configure_file("${DOXY_FOOTER_CONFIG_TEMPLATE_FILE}" "${DOXY_FOOTER_GEN_OUTPUT_FILE}" @ONLY)
    configure_file("${DOXY_PUBLIC_CONFIG_TEMPLATE_FILE}" "${DOXY_PUBLIC_CONFIG_GEN_OUTPUT_FILE}" @ONLY)
    configure_file("${DOXY_INTERNAL_CONFIG_TEMPLATE_FILE}" "${DOXY_INTERNAL_CONFIG_GEN_OUTPUT_FILE}" @ONLY)

    # Build targets.
    add_custom_target(docs_public
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOXY_PUBLIC_OUTPUT_DIRECTORY}"
            COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXY_PUBLIC_CONFIG_GEN_OUTPUT_FILE}"
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            DEPENDS
            "${DOXY_PUBLIC_CONFIG_TEMPLATE_FILE}"
            ${DOXY_PUBLIC_INPUT_FILES}
            COMMENT "Generating docs_public documentation with Doxygen"
            VERBATIM
    )

    add_custom_target(docs_internal
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOXY_INTERNAL_OUTPUT_DIRECTORY}"
            COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXY_INTERNAL_CONFIG_GEN_OUTPUT_FILE}"
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            DEPENDS
            "${DOXY_INTERNAL_CONFIG_TEMPLATE_FILE}"
            ${DOXY_INTERNAL_INPUT_FILES}
            COMMENT "Generating docs_internal documentation with Doxygen"
            VERBATIM
    )

    add_custom_target(docs DEPENDS docs_public docs_internal)
endif ()
