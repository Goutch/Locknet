cmake_minimum_required(VERSION 3.24)
project(locknet)

include_directories(dependencies/fpm/include)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(dependencies/enet)
include_directories(dependencies/enet/include)

add_library(${PROJECT_NAME} SHARED
        src/Server.cpp
        src/Client.cpp
        )
set(ENET_LIBRARIES enet winmm ws2_32)

target_link_libraries(${PROJECT_NAME} ${ENET_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/enet/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/fpm/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/utils-collection)