cmake_minimum_required(VERSION 3.12)
project(handy_variant VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_library(handy_variant INTERFACE)
target_include_directories(handy_variant INTERFACE ./include/)

# -----------------------------------------------------------------------------
# tests
# -----------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)