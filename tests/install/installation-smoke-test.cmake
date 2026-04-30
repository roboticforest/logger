cmake_minimum_required(VERSION 4.0)

file(REMOVE_RECURSE "${dvlogger_install_prefix}" "${dvlogger_consumer_binary_dir}")
file(MAKE_DIRECTORY "${dvlogger_install_prefix}")

function(run_or_fail error_message)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE result COMMAND_ECHO STDOUT)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${error_message}")
    endif()
endfunction()

set(dvlogger_config_args)
if(DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL "")
    list(APPEND dvlogger_config_args --config "${dvlogger_build_config}")
endif()

set(dvlogger_generator_args)
if(DEFINED dvlogger_generator AND NOT "${dvlogger_generator}" STREQUAL "")
    list(APPEND dvlogger_generator_args -G "${dvlogger_generator}")
endif()

set(dvlogger_toolchain_args)
if(WIN32)
    file(STRINGS "${dvlogger_binary_dir}/CMakeCache.txt" cxx_line REGEX "^CMAKE_CXX_COMPILER:FILEPATH=" LIMIT_COUNT 1)
    file(STRINGS "${dvlogger_binary_dir}/CMakeCache.txt" rc_line REGEX "^CMAKE_RC_COMPILER:FILEPATH=" LIMIT_COUNT 1)
    file(STRINGS "${dvlogger_binary_dir}/CMakeCache.txt" mt_line REGEX "^CMAKE_MT:FILEPATH=" LIMIT_COUNT 1)
    string(REPLACE "CMAKE_CXX_COMPILER:FILEPATH=" "" cxx "${cxx_line}")
    string(REPLACE "CMAKE_RC_COMPILER:FILEPATH=" "" rc "${rc_line}")
    string(REPLACE "CMAKE_MT:FILEPATH=" "" mt "${mt_line}")

    if(NOT "${cxx}" STREQUAL "")
        list(APPEND dvlogger_toolchain_args "-DCMAKE_CXX_COMPILER:FILEPATH=${cxx}")
    endif()
    if(NOT "${rc}" STREQUAL "")
        list(APPEND dvlogger_toolchain_args "-DCMAKE_RC_COMPILER:FILEPATH=${rc}")
    endif()
    if(NOT "${mt}" STREQUAL "")
        list(APPEND dvlogger_toolchain_args "-DCMAKE_MT:FILEPATH=${mt}")
    endif()

    if((NOT DEFINED ENV{LIB} OR "$ENV{LIB}" STREQUAL "" OR NOT DEFINED ENV{INCLUDE} OR "$ENV{INCLUDE}" STREQUAL "")
       AND NOT "${cxx}" STREQUAL "" AND NOT "${rc}" STREQUAL "")
        get_filename_component(msvc_bin "${cxx}" DIRECTORY)
        get_filename_component(msvc_root "${msvc_bin}/../../.." ABSOLUTE)
        get_filename_component(sdk_arch_bin "${rc}" DIRECTORY)
        get_filename_component(sdk_ver_bin "${sdk_arch_bin}/.." ABSOLUTE)
        get_filename_component(sdk_ver "${sdk_ver_bin}" NAME)
        get_filename_component(sdk_bin "${sdk_ver_bin}/.." ABSOLUTE)
        get_filename_component(sdk_root "${sdk_bin}/.." ABSOLUTE)
        if(NOT DEFINED ENV{LIB} OR "$ENV{LIB}" STREQUAL "")
            set(ENV{LIB} "${msvc_root}/lib/x64;${sdk_root}/Lib/${sdk_ver}/um/x64;${sdk_root}/Lib/${sdk_ver}/ucrt/x64")
        endif()
        if(NOT DEFINED ENV{INCLUDE} OR "$ENV{INCLUDE}" STREQUAL "")
            set(ENV{INCLUDE}
                "${msvc_root}/include;${sdk_root}/Include/${sdk_ver}/ucrt;${sdk_root}/Include/${sdk_ver}/shared;${sdk_root}/Include/${sdk_ver}/um;${sdk_root}/Include/${sdk_ver}/winrt"
            )
        endif()
    endif()
endif()

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

if(WIN32)
    set(ENV{PATH} "${dvlogger_install_prefix}/bin;$ENV{PATH}")
endif()

set(dvlogger_consumer_executable "${dvlogger_consumer_binary_dir}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
if(DEFINED dvlogger_build_config AND NOT "${dvlogger_build_config}" STREQUAL "")
    set(config_executable "${dvlogger_consumer_binary_dir}/${dvlogger_build_config}/dvlogger_install_smoke${CMAKE_EXECUTABLE_SUFFIX}")
    if(EXISTS "${config_executable}")
        set(dvlogger_consumer_executable "${config_executable}")
    endif()
endif()

run_or_fail("The install smoke test consumer executable failed." "${dvlogger_consumer_executable}")
