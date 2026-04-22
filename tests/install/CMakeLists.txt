cmake_minimum_required(VERSION 4.0)
project(dvlogger_install_smoke LANGUAGES CXX)

find_package(DVLogger CONFIG REQUIRED)

add_executable(dvlogger_install_smoke main.cpp)
target_compile_features(dvlogger_install_smoke PRIVATE cxx_std_20)
target_link_libraries(dvlogger_install_smoke PRIVATE DV::Logger)
