cmake_minimum_required(VERSION 4.0)

file(REMOVE_RECURSE "${dvlogger_install_prefix}" "${dvlogger_consumer_binary_dir}")
file(MAKE_DIRECTORY "${dvlogger_install_prefix}")

set(dvlogger_config_args)
if(DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL "")
    set(dvlogger_config_args --config "${dvlogger_build_config}")
endif()

set(dvlogger_generator_args)
if(DEFINED dvlogger_generator AND NOT "${dvlogger_generator}" STREQUAL "")
    set(dvlogger_generator_args -G "${dvlogger_generator}")
endif()

function(run_or_fail error_message)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result COMMAND_ECHO STDOUT)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${error_message}")
    endif()
endfunction()

run_or_fail(
    "Failed to install DVLogger for install smoke testing."
    "${CMAKE_COMMAND}" --install "${dvlogger_binary_dir}" --prefix "${dvlogger_install_prefix}" ${dvlogger_config_args}
)

run_or_fail(
    "Failed to configure the install smoke test consumer."
    "${CMAKE_COMMAND}"
    -S "${dvlogger_consumer_source_dir}"
    -B "${dvlogger_consumer_binary_dir}"
    ${dvlogger_generator_args}
    "-DCMAKE_PREFIX_PATH=${dvlogger_install_prefix}"
)
run_or_fail(
    "Failed to build the install smoke test consumer."
    "${CMAKE_COMMAND}" --build "${dvlogger_consumer_binary_dir}" ${dvlogger_config_args}
)

set(dvlogger_consumer_executable "${dvlogger_consumer_binary_dir}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
if(WIN32 AND DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL "")
    set(dvlogger_consumer_executable "${dvlogger_consumer_binary_dir}/${dvlogger_build_config}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
endif()

run_or_fail("The install smoke test consumer executable failed." "${dvlogger_consumer_executable}")
