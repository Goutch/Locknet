cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)

project(App CXX)
include_directories(".")

set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
add_subdirectory("locknet")
add_subdirectory("client")