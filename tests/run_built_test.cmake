cmake_minimum_required(VERSION 4.0)

if(NOT DEFINED dvlogger_binary_dir OR NOT DEFINED dvlogger_test_target OR NOT DEFINED dvlogger_test_executable)
    message(FATAL_ERROR "run_built_test.cmake requires dvlogger_binary_dir, dvlogger_test_target, and dvlogger_test_executable.")
endif()

if(DEFINED dvlogger_env_include AND NOT "${dvlogger_env_include}" STREQUAL "")
    set(ENV{INCLUDE} "${dvlogger_env_include}")
endif()
if(DEFINED dvlogger_env_lib AND NOT "${dvlogger_env_lib}" STREQUAL "")
    set(ENV{LIB} "${dvlogger_env_lib}")
endif()
if(DEFINED dvlogger_env_libpath AND NOT "${dvlogger_env_libpath}" STREQUAL "")
    set(ENV{LIBPATH} "${dvlogger_env_libpath}")
endif()

set(dvlogger_config_args)
if(DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL "")
    set(dvlogger_config_args --config "${dvlogger_build_config}")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        --build "${dvlogger_binary_dir}"
        ${dvlogger_config_args}
        --target "${dvlogger_test_target}"
    RESULT_VARIABLE dvlogger_build_result
    COMMAND_ECHO STDOUT
)
if(NOT dvlogger_build_result EQUAL 0)
    message(FATAL_ERROR "Failed to build test target '${dvlogger_test_target}'.")
endif()

execute_process(
    COMMAND "${dvlogger_test_executable}"
    RESULT_VARIABLE dvlogger_run_result
    COMMAND_ECHO STDOUT
)
if(NOT dvlogger_run_result EQUAL 0)
    message(FATAL_ERROR "Test executable failed: ${dvlogger_test_executable}")
endif()
