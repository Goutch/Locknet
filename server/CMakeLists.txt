cmake_minimum_required(VERSION 3.24)
project(locknetServer)


set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        main.cpp)

target_link_libraries(${PROJECT_NAME} locknet)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})