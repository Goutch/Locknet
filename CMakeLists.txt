cmake_minimum_required(VERSION 3.24)
project(LockNet)

include_directories(dependencies/fpm/include)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(dependencies/enet)
include_directories(dependencies/enet/include)

add_executable(LockNet
        Server.cpp
        Server.h
        Client.cpp
        Client.h
        LockNet.h
        Packet.h)
set(ENET_LIBRARIES enet winmm ws2_32)

target_link_libraries(${PROJECT_NAME} ${ENET_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC dependencies/enet/include)
target_include_directories(${PROJECT_NAME} PUBLIC dependencies/fpm/include)