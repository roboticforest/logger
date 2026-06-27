# Builds a logger test executable and registers it with CTest.
#
# test_target_name: Name to use for the test executable and its matching CTest entry.
# test_dir: Folder under tests containing the test main.cpp file.

function(add_logger_test test_target_name test_dir)
    add_executable("${test_target_name}" "${PROJECT_SOURCE_DIR}/tests/${test_dir}/main.cpp")
    target_link_libraries("${test_target_name}" PRIVATE DV::LoggerStatic)
    set_property(TARGET "${test_target_name}" PROPERTY FOLDER "DVLogger Tests")
    add_test(NAME "${test_target_name}" COMMAND "$<TARGET_FILE:${test_target_name}>")
endfunction()
