cmake_minimum_required(VERSION 3.25)
project(kill_nvda_remotely CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

add_executable(client WIN32 ${SRC_DIR}/client.cpp)
target_link_libraries(client ws2_32 shlwapi)

add_executable(server WIN32 ${SRC_DIR}/server.cpp)
target_link_libraries(server ws2_32 shlwapi)
