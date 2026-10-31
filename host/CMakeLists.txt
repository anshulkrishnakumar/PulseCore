cmake_minimum_required(VERSION 3.20)

project(PulseCoreHost LANGUAGES CXX)
find_package(SQLite3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Main host application
add_executable(pulsecore_host
    src/main.cpp
    src/telemetry_parser.cpp
    src/database.cpp
)

target_include_directories(pulsecore_host PRIVATE include)

target_compile_options(pulsecore_host PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(pulsecore_host PRIVATE SQLite::SQLite3)

# GoogleTest
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

# Unit tests
enable_testing()

add_executable(pulsecore_tests
    tests/test_telemetry_parser.cpp
    tests/test_database.cpp
    src/telemetry_parser.cpp
    src/database.cpp
)

target_include_directories(pulsecore_tests PRIVATE include)

target_link_libraries(pulsecore_tests PRIVATE
    GTest::gtest_main
    SQLite::SQLite3
)

include(GoogleTest)
gtest_discover_tests(pulsecore_tests)