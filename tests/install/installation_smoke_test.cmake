cmake_minimum_required(VERSION 4.0)

# Installation smoke test flow:
# 1) Build + install DVLogger into an isolated prefix.
# 2) Configure + build a tiny consumer that uses find_package(DVLogger CONFIG).
# 3) Run the consumer executable to verify the installed package is usable.

set(dvlogger_consumer_source_dir "${dvlogger_source_dir}/tests/install")

# Reset scratch directories so every run starts from a clean state.
file(REMOVE_RECURSE "${dvlogger_install_prefix}" "${dvlogger_consumer_binary_dir}")
file(MAKE_DIRECTORY "${dvlogger_install_prefix}")

# Pass --config only for multi-config generators (Visual Studio, Ninja Multi-Config).
set(dvlogger_config_args)
if(DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL "")
    set(dvlogger_config_args --config "${dvlogger_build_config}")
endif()

# Restore compiler environment when CTest is launched without a developer shell.
if(DEFINED dvlogger_env_include AND NOT "${dvlogger_env_include}" STREQUAL "")
    set(ENV{INCLUDE} "${dvlogger_env_include}")
endif()
if(DEFINED dvlogger_env_lib AND NOT "${dvlogger_env_lib}" STREQUAL "")
    set(ENV{LIB} "${dvlogger_env_lib}")
endif()
if(DEFINED dvlogger_env_libpath AND NOT "${dvlogger_env_libpath}" STREQUAL "")
    set(ENV{LIBPATH} "${dvlogger_env_libpath}")
endif()

# Keep the external consumer on the same generator as the producer project.
set(dvlogger_generator_args)
if(DEFINED dvlogger_generator AND NOT "${dvlogger_generator}" STREQUAL "")
    set(dvlogger_generator_args -G "${dvlogger_generator}")
endif()

# Execute a command and fail the test immediately if it returns a non-zero exit code.
function(dvlogger_run_or_fail error_message)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result COMMAND_ECHO STDOUT)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${error_message}")
    endif()
endfunction()

# Build and install the producer project under test.
if(NOT DEFINED dvlogger_rebuild_before_install)
    set(dvlogger_rebuild_before_install OFF)
endif()
if(DEFINED dvlogger_rebuild_before_install AND dvlogger_rebuild_before_install)
    dvlogger_run_or_fail(
        "Failed to build dvlogger for install smoke testing."
        "${CMAKE_COMMAND}"
        --build "${dvlogger_binary_dir}"
        ${dvlogger_config_args}
        --target dvlogger_shared dvlogger_static
    )
endif()
dvlogger_run_or_fail(
        "Failed to install DVLogger for install smoke testing."
    "${CMAKE_COMMAND}" --install "${dvlogger_binary_dir}" --prefix "${dvlogger_install_prefix}" ${dvlogger_config_args}
)

# Configure and build an external consumer against the installed package.
dvlogger_run_or_fail(
    "Failed to configure the install smoke test consumer."
    "${CMAKE_COMMAND}"
    -S "${dvlogger_consumer_source_dir}"
    -B "${dvlogger_consumer_binary_dir}"
    ${dvlogger_generator_args}
    "-DCMAKE_PREFIX_PATH=${dvlogger_install_prefix}"
)
dvlogger_run_or_fail(
    "Failed to build the install smoke test consumer."
    "${CMAKE_COMMAND}" --build "${dvlogger_consumer_binary_dir}" ${dvlogger_config_args}
)

# Multi-config generators place executables under a config subdirectory.
set(dvlogger_consumer_executable "${dvlogger_consumer_binary_dir}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
if(WIN32 AND DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL "")
    set(dvlogger_consumer_executable "${dvlogger_consumer_binary_dir}/${dvlogger_build_config}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
endif()

# Final verification: the consumer must run successfully.
dvlogger_run_or_fail("The install smoke test consumer executable failed." "${dvlogger_consumer_executable}")
