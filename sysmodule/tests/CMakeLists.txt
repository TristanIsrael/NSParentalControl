cmake_minimum_required(VERSION 3.15)
project(SysmoduleTests)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(NO_SWITCH_H=1)

# src
add_library(SysModule 
    ../source/database/history_entry.cpp
    ../source/database/stream_reader.cpp
    ../source/database/stream_writer.cpp
    ../source/types.cpp
) 

add_executable(tests 
    test_streams.cpp 
    test_history.cpp
)

target_compile_definitions(tests PRIVATE
    NO_SWITCH_H=1
)

# tests
enable_testing()

# Catch2 
find_package(Catch2 3 REQUIRED)
include_directories(/opt/homebrew/include ../source)
target_link_libraries(tests PRIVATE SysModule Catch2::Catch2WithMain)
include(CTest)
include(Catch)

catch_discover_tests(tests)