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

set(dvlogger_toolchain_args)
if(WIN32)
    file(STRINGS "${dvlogger_binary_dir}/CMakeCache.txt" dvlogger_cxx_compiler_line REGEX "^CMAKE_CXX_COMPILER:FILEPATH=" LIMIT_COUNT 1)
    file(STRINGS "${dvlogger_binary_dir}/CMakeCache.txt" dvlogger_rc_compiler_line REGEX "^CMAKE_RC_COMPILER:FILEPATH=" LIMIT_COUNT 1)
    file(STRINGS "${dvlogger_binary_dir}/CMakeCache.txt" dvlogger_mt_tool_line REGEX "^CMAKE_MT:FILEPATH=" LIMIT_COUNT 1)

    string(REPLACE "CMAKE_CXX_COMPILER:FILEPATH=" "" dvlogger_cxx_compiler "${dvlogger_cxx_compiler_line}")
    string(REPLACE "CMAKE_RC_COMPILER:FILEPATH=" "" dvlogger_rc_compiler "${dvlogger_rc_compiler_line}")
    string(REPLACE "CMAKE_MT:FILEPATH=" "" dvlogger_mt_tool "${dvlogger_mt_tool_line}")

    if(NOT "${dvlogger_cxx_compiler}" STREQUAL "" AND NOT "${dvlogger_rc_compiler}" STREQUAL "")
        get_filename_component(dvlogger_msvc_bin_dir "${dvlogger_cxx_compiler}" DIRECTORY)
        get_filename_component(dvlogger_msvc_root "${dvlogger_msvc_bin_dir}/../../.." ABSOLUTE)

        get_filename_component(dvlogger_sdk_arch_bin_dir "${dvlogger_rc_compiler}" DIRECTORY)
        get_filename_component(dvlogger_sdk_version_bin_dir "${dvlogger_sdk_arch_bin_dir}/.." ABSOLUTE)
        get_filename_component(dvlogger_windows_sdk_version "${dvlogger_sdk_version_bin_dir}" NAME)
        get_filename_component(dvlogger_sdk_bin_dir "${dvlogger_sdk_version_bin_dir}/.." ABSOLUTE)
        get_filename_component(dvlogger_windows_sdk_root "${dvlogger_sdk_bin_dir}/.." ABSOLUTE)

        set(ENV{LIB}
            "${dvlogger_msvc_root}/lib/x64;${dvlogger_windows_sdk_root}/Lib/${dvlogger_windows_sdk_version}/um/x64;${dvlogger_windows_sdk_root}/Lib/${dvlogger_windows_sdk_version}/ucrt/x64;$ENV{LIB}"
        )
        set(ENV{INCLUDE}
            "${dvlogger_msvc_root}/include;${dvlogger_windows_sdk_root}/Include/${dvlogger_windows_sdk_version}/ucrt;${dvlogger_windows_sdk_root}/Include/${dvlogger_windows_sdk_version}/shared;${dvlogger_windows_sdk_root}/Include/${dvlogger_windows_sdk_version}/um;${dvlogger_windows_sdk_root}/Include/${dvlogger_windows_sdk_version}/winrt;$ENV{INCLUDE}"
        )
    endif()

    if(NOT "${dvlogger_cxx_compiler}" STREQUAL "")
        set(dvlogger_toolchain_args "-DCMAKE_CXX_COMPILER:FILEPATH=${dvlogger_cxx_compiler}")
    endif()
    if(NOT "${dvlogger_rc_compiler}" STREQUAL "")
        list(APPEND dvlogger_toolchain_args "-DCMAKE_RC_COMPILER:FILEPATH=${dvlogger_rc_compiler}")
    endif()
    if(NOT "${dvlogger_mt_tool}" STREQUAL "")
        list(APPEND dvlogger_toolchain_args "-DCMAKE_MT:FILEPATH=${dvlogger_mt_tool}")
    endif()
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
    ${dvlogger_toolchain_args}
    "-DCMAKE_PREFIX_PATH=${dvlogger_install_prefix}"
)
run_or_fail(
    "Failed to build the install smoke test consumer."
    "${CMAKE_COMMAND}" --build "${dvlogger_consumer_binary_dir}" ${dvlogger_config_args}
)

set(dvlogger_consumer_executable "${dvlogger_consumer_binary_dir}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
if(WIN32)
    set(ENV{PATH} "${dvlogger_install_prefix}/bin;$ENV{PATH}")
endif()
if(WIN32
   AND DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL ""
   AND DEFINED dvlogger_generator
   AND ("${dvlogger_generator}" MATCHES "Visual Studio" OR "${dvlogger_generator}" STREQUAL "Ninja Multi-Config"))
    set(dvlogger_consumer_executable "${dvlogger_consumer_binary_dir}/${dvlogger_build_config}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
endif()

run_or_fail("The install smoke test consumer executable failed." "${dvlogger_consumer_executable}")
