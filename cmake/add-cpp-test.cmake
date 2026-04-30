# Builds a C++ test executable, links it to a library dependency, and registers it with CTest.
#
# test_target_name: Name to use for the test executable and its matching CTest entry.
# test_src_file: A C++ source file to build the test executable from.
# test_label_list: Semicolon-separated CTest labels to assign to the test.
# linked_library_target: Library target that the test executable is built and run against.

function(add_cpp_test test_target_name test_src_file test_label_list linked_library_target)
    add_executable("${test_target_name}" "${test_src_file}")
    target_link_libraries("${test_target_name}" PRIVATE "${linked_library_target}")
    add_test(NAME "${test_target_name}" COMMAND "$<TARGET_FILE:${test_target_name}>")
    set_tests_properties("${test_target_name}" PROPERTIES LABELS "${test_label_list}")
endfunction()
