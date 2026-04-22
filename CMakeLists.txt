# All rights reserved (c) 2020-2026 David Vitez
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 4.0)
project(DVLogger VERSION 0.90.0.0 LANGUAGES CXX)

####################################################################################################
# Project Setup
####################################################################################################

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

find_package(Threads REQUIRED)

option(DVLOGGER_BUILD_PUBLIC_DOCS "Build API documentation with Doxygen." ON)
option(DVLOGGER_BUILD_INTERNAL_DOCS "Build private developer documentation with Doxygen." ON)
option(DVLOGGER_ENABLE_MANUAL_TESTS "Enable manual/stress tests in CTest." OFF)
option(DVLOGGER_ENABLE_INSTALL "Enable install/export rules." ON)

set(DVLOGGER_LIBRARY_SOURCES "${PROJECT_SOURCE_DIR}/logger.cpp")
set(DVLOGGER_MODULE_FILES "${PROJECT_SOURCE_DIR}/logger.cppm")
set(DVLOGGER_VERSION_DEFINITIONS
    DVLOGGER_VERSION_STRING="${PROJECT_VERSION}"
    DVLOGGER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    DVLOGGER_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    DVLOGGER_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    DVLOGGER_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)

####################################################################################################
# Library Targets
####################################################################################################

add_library(dvlogger_shared SHARED)
target_compile_features(dvlogger_shared PUBLIC cxx_std_20)
target_sources(dvlogger_shared
    PRIVATE
        ${DVLOGGER_LIBRARY_SOURCES}
    PUBLIC
        FILE_SET dvlogger_modules
        TYPE CXX_MODULES
        BASE_DIRS "${PROJECT_SOURCE_DIR}"
        FILES ${DVLOGGER_MODULE_FILES}
)
target_compile_definitions(dvlogger_shared PUBLIC ${DVLOGGER_VERSION_DEFINITIONS})
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
        ${DVLOGGER_LIBRARY_SOURCES}
    PUBLIC
        FILE_SET dvlogger_modules
        TYPE CXX_MODULES
        BASE_DIRS "${PROJECT_SOURCE_DIR}"
        FILES ${DVLOGGER_MODULE_FILES}
)
target_compile_definitions(dvlogger_static PUBLIC ${DVLOGGER_VERSION_DEFINITIONS})
target_link_libraries(dvlogger_static PUBLIC Threads::Threads)
set_target_properties(dvlogger_static PROPERTIES
    OUTPUT_NAME "dvlogger_static"
    EXPORT_NAME "LoggerStatic"
)
add_library(DV::LoggerStatic ALIAS dvlogger_static)

####################################################################################################
# Install/Export
####################################################################################################

if(DVLOGGER_ENABLE_INSTALL)
    set(dvlogger_package_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
    set(dvlogger_package_config "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
    set(dvlogger_package_version "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${dvlogger_package_config}"
        INSTALL_DESTINATION "${dvlogger_package_dir}"
    )

    write_basic_package_version_file(
        "${dvlogger_package_version}"
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
        FILE_SET dvlogger_modules DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/modules"
    )

    install(EXPORT DVLoggerTargets
        FILE "${PROJECT_NAME}Targets.cmake"
        NAMESPACE DV::
        DESTINATION "${dvlogger_package_dir}"
    )

    install(FILES
        "${dvlogger_package_config}"
        "${dvlogger_package_version}"
        DESTINATION "${dvlogger_package_dir}"
    )
endif()

####################################################################################################
# Tests
####################################################################################################

if(BUILD_TESTING)
    set(DVLOGGER_UNIT_TEST_TARGETS
        dvlogger_smoke_test
        dvlogger_format_test
        dvlogger_stream_test
        dvlogger_thread_test
        dvlogger_failure_mode_test
    )
    set(DVLOGGER_MANUAL_TEST_TARGETS
        dvlogger_stress_manual_test
        dvlogger_dead_stream_manual_test
    )

    string(REPLACE ";" "\\;" DVLOGGER_TEST_ENV_INCLUDE "$ENV{INCLUDE}")
    string(REPLACE ";" "\\;" DVLOGGER_TEST_ENV_LIB "$ENV{LIB}")
    string(REPLACE ";" "\\;" DVLOGGER_TEST_ENV_LIBPATH "$ENV{LIBPATH}")

    function(dvlogger_add_cpp_test target_name source_file test_labels)
        add_executable("${target_name}" "${source_file}")
        target_compile_features("${target_name}" PRIVATE cxx_std_20)
        target_link_libraries("${target_name}" PRIVATE dvlogger_static)
        add_test(
            NAME "${target_name}"
            COMMAND
                "${CMAKE_COMMAND}"
                "-Ddvlogger_binary_dir=${PROJECT_BINARY_DIR}"
                "-Ddvlogger_test_target=${target_name}"
                "-Ddvlogger_test_executable=$<TARGET_FILE:${target_name}>"
                "-Ddvlogger_build_config=$<CONFIG>"
                "-Ddvlogger_env_include=${DVLOGGER_TEST_ENV_INCLUDE}"
                "-Ddvlogger_env_lib=${DVLOGGER_TEST_ENV_LIB}"
                "-Ddvlogger_env_libpath=${DVLOGGER_TEST_ENV_LIBPATH}"
                "-P"
                "${PROJECT_SOURCE_DIR}/tests/run_built_test.cmake"
        )
        set_tests_properties("${target_name}" PROPERTIES LABELS "${test_labels}")

        add_custom_target("run_${target_name}"
            COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -R "^${target_name}$"
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            USES_TERMINAL
            DEPENDS "${target_name}"
        )
    endfunction()

    dvlogger_add_cpp_test(
        dvlogger_smoke_test
        "${PROJECT_SOURCE_DIR}/tests/logging_smoke_test.cpp"
        "unit;smoke"
    )
    dvlogger_add_cpp_test(
        dvlogger_format_test
        "${PROJECT_SOURCE_DIR}/tests/format_tests.cpp"
        "unit;format"
    )
    dvlogger_add_cpp_test(
        dvlogger_stream_test
        "${PROJECT_SOURCE_DIR}/tests/stream_tests.cpp"
        "unit;stream"
    )
    dvlogger_add_cpp_test(
        dvlogger_thread_test
        "${PROJECT_SOURCE_DIR}/tests/simple_threading_tests.cpp"
        "unit;thread"
    )
    dvlogger_add_cpp_test(
        dvlogger_failure_mode_test
        "${PROJECT_SOURCE_DIR}/tests/failure_mode_tests.cpp"
        "unit;failure_mode"
    )

    if(DVLOGGER_ENABLE_MANUAL_TESTS)
        dvlogger_add_cpp_test(
            dvlogger_stress_manual_test
            "${PROJECT_SOURCE_DIR}/tests/manual/threading_stress_manual_test.cpp"
            "manual;stress"
        )
        dvlogger_add_cpp_test(
            dvlogger_dead_stream_manual_test
            "${PROJECT_SOURCE_DIR}/tests/manual/dead_stream_manual_test.cpp"
            "manual;failure_mode"
        )
    endif()

    add_custom_target(dvlogger_tests_unit
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -L unit
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        USES_TERMINAL
        DEPENDS ${DVLOGGER_UNIT_TEST_TARGETS}
    )
    add_custom_target(dvlogger_tests_normal DEPENDS dvlogger_tests_unit)
    set(DVLOGGER_ALL_TEST_TARGETS ${DVLOGGER_UNIT_TEST_TARGETS})

    if(DVLOGGER_ENABLE_INSTALL)
        add_test(
            NAME dvlogger_installation_smoke_test
            COMMAND
                "${CMAKE_COMMAND}"
                "-Ddvlogger_binary_dir=${PROJECT_BINARY_DIR}"
                "-Ddvlogger_source_dir=${PROJECT_SOURCE_DIR}"
                "-Ddvlogger_install_prefix=${PROJECT_BINARY_DIR}/install-smoke/prefix"
                "-Ddvlogger_consumer_binary_dir=${PROJECT_BINARY_DIR}/install-smoke/build"
                "-Ddvlogger_build_config=$<CONFIG>"
                "-Ddvlogger_rebuild_before_install=ON"
                "-Ddvlogger_generator=${CMAKE_GENERATOR}"
                "-Ddvlogger_env_include=${DVLOGGER_TEST_ENV_INCLUDE}"
                "-Ddvlogger_env_lib=${DVLOGGER_TEST_ENV_LIB}"
                "-Ddvlogger_env_libpath=${DVLOGGER_TEST_ENV_LIBPATH}"
                "-P"
                "${PROJECT_SOURCE_DIR}/tests/install/installation_smoke_test.cmake"
        )
        set_tests_properties(dvlogger_installation_smoke_test PROPERTIES LABELS "integration;install")

        add_custom_target(run_dvlogger_installation_smoke_test
            COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -R "^dvlogger_installation_smoke_test$"
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            USES_TERMINAL
            DEPENDS dvlogger_shared dvlogger_static
        )
        list(APPEND DVLOGGER_ALL_TEST_TARGETS dvlogger_shared dvlogger_static)
    endif()

    if(DVLOGGER_ENABLE_MANUAL_TESTS)
        list(APPEND DVLOGGER_ALL_TEST_TARGETS ${DVLOGGER_MANUAL_TEST_TARGETS})
        add_custom_target(dvlogger_tests_manual
            COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure -L manual
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            USES_TERMINAL
            DEPENDS ${DVLOGGER_MANUAL_TEST_TARGETS}
        )
        add_custom_target(dvlogger_tests_all
            COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            USES_TERMINAL
            DEPENDS ${DVLOGGER_ALL_TEST_TARGETS}
        )
    else()
        add_custom_target(dvlogger_tests_manual
            COMMAND "${CMAKE_COMMAND}" -E echo "Manual tests are disabled."
            USES_TERMINAL
        )
        add_custom_target(dvlogger_tests_all
            COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
            USES_TERMINAL
            DEPENDS ${DVLOGGER_ALL_TEST_TARGETS}
        )
    endif()
endif()

####################################################################################################
# Docs
####################################################################################################

if(DVLOGGER_BUILD_PUBLIC_DOCS)
    find_package(Doxygen REQUIRED COMPONENTS doxygen)
    set(DVLOGGER_DOC_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DVLOGGER_DOC_EXCLUDE_PATTERNS "*/.git/* */.svn/* */.hg/* */CMakeFiles/* */_CPack_Packages/* DartConfiguration.tcl CMakeLists.txt CMakeCache.txt")
    string(TIMESTAMP DVLOGGER_DOC_COPYRIGHT_YEAR "%Y")
    set(DVLOGGER_DOC_COPYRIGHT_TEXT "Copyright (c) ${DVLOGGER_DOC_COPYRIGHT_YEAR} David Vitez & DV::Logger contributors.")
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
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DVLOGGER_DOC_PUBLIC_OUTPUT_DIRECTORY}"
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
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DVLOGGER_DOC_INTERNAL_OUTPUT_DIRECTORY}"
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
