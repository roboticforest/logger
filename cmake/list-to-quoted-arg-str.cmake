# Converts a CMake list into a string of quoted, space-separated items
# appropriate for tools like Doxygen. This function assumes list items
# do not contain literal quote characters.
#
# For example:
# Input List: Some doc.txt;/a/file/path;/some other/file path/
# Output Str: "\"Some doc.txt\" \"/a/file/path\" \"/some other/file path/\""

function(list_to_quoted_arg_str input_list_var output_string_var)
    set(_list_items ${${input_list_var}})
    list(TRANSFORM _list_items PREPEND "\"")
    list(TRANSFORM _list_items APPEND "\"")
    string(JOIN " " _quoted_items ${_list_items})
    set(${output_string_var} "${_quoted_items}" PARENT_SCOPE)
endfunction()
