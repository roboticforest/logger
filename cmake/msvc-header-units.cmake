function(add_msvc_header_units target_name visibility)
    if (NOT MSVC)
        return()
    endif ()

    if (NOT TARGET "${target_name}")
        message(FATAL_ERROR "add_msvc_header_units called for unknown target: ${target_name}")
    endif ()

    if (NOT visibility MATCHES "^(PRIVATE|PUBLIC|INTERFACE)$")
        message(FATAL_ERROR "add_msvc_header_units visibility must be PRIVATE, PUBLIC, or INTERFACE.")
    endif ()

    foreach(header_name IN LISTS ARGN)
        string(MAKE_C_IDENTIFIER "${target_name}_${header_name}" header_unit_id)

        set(header_unit_dir "${CMAKE_CURRENT_BINARY_DIR}/msvc-header-units/${target_name}/$<CONFIG>")
        set(header_unit_ifc "${header_unit_dir}/${header_unit_id}.ifc")
        set(header_unit_target "${header_unit_id}_header_unit")

        add_custom_command(
                OUTPUT "${header_unit_ifc}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${header_unit_dir}"
                COMMAND ${CMAKE_CXX_COMPILER}
                        /nologo
                        /std:c++20
                        /EHsc
                        /MD$<$<CONFIG:Debug>:d>
                        /exportHeader
                        /headerName:angle "${header_name}"
                        /ifcOutput "${header_unit_ifc}"
                COMMENT "Building MSVC header unit <${header_name}> for ${target_name}"
                VERBATIM
        )

        add_custom_target("${header_unit_target}" DEPENDS "${header_unit_ifc}")
        add_dependencies("${target_name}" "${header_unit_target}")

        target_compile_options("${target_name}"
                ${visibility}
                "SHELL:/headerUnit:angle ${header_name}=${header_unit_ifc}"
        )
    endforeach()
endfunction()
