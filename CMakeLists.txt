cmake_minimum_required(VERSION 3.25)
project(kill_nvda_remotely CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

add_executable(client.exe WIN32 ${SRC_DIR}/client.cpp)
target_link_libraries(client.exe ws2_32 shlwapi)

add_executable(server.exe WIN32 ${SRC_DIR}/server.cpp)
target_link_libraries(server.exe ws2_32 shlwapi)

add_custom_target(package ALL
	COMMAND powershell -Command "Compress-Archive -Force -Path 'client.exe', 'server.exe', 'config.ini', 'readme.txt' -DestinationPath '${CMAKE_BINARY_DIR}/kill_nvda_remotely.zip'"
	DEPENDS client.exe server.exe
	COMMENT "Packaging binaries into kill_nvda_remotely.zip"
)
