cmake_minimum_required(VERSION 3.24)
project(locknet)


set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(LOCKNET_BUILD)


#-----------------fpm -----------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/fpm/include)

#-----------------enet -----------------------------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/enet)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/enet/include)

#------------Steamworks -------------------------------
list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
option(USE_STEAMWORKS "Enable Steamworks integration" ON)
if (USE_STEAMWORKS)
    find_package(STEAMWORKS)
endif ()
#--------------------------------------------

add_library(${PROJECT_NAME} SHARED
        src/Server.cpp
        src/Client.cpp)
set(ENET_LIBRARIES enet winmm ws2_32)
if (STEAMWORKS_FOUND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_STEAMWORKS=1)
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/steamworks-sdk/public)
endif ()
target_link_libraries(${PROJECT_NAME} ${ENET_LIBRARIES} ${STEAMWORKS_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/enet/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/fpm/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/utils-collection)

if (Steamworks_FOUND)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${STEAMWORKS_LIB}"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif ()