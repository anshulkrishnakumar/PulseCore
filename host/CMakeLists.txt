cmake_minimum_required(VERSION 3.20)

project(PulseCoreHost LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pulsecore_host
    src/main.cpp
)

target_include_directories(pulsecore_host PRIVATE include)

target_compile_options(pulsecore_host PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)