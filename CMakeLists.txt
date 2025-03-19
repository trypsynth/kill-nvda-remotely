cmake_minimum_required(VERSION 3.25.0)
project(kill_nvda_remotely LANGUAGES C)

add_executable(client WIN32 client.c)
target_link_libraries(client PRIVATE ws2_32 shlwapi)

add_executable(server WIN32 server.c)
target_link_libraries(server PRIVATE ws2_32 shlwapi)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}")
set(ZIP_FILE "${CMAKE_BINARY_DIR}/kill_nvda_remotely.zip")

add_custom_target(package ALL
	COMMAND ${CMAKE_COMMAND} -E echo "Packaging binaries into ${ZIP_FILE}"
	COMMAND powershell -Command "Compress-Archive -Force -Path `
		'${OUTPUT_DIR}/client.exe', `
		'${OUTPUT_DIR}/server.exe', `
		'${CMAKE_SOURCE_DIR}/config.ini', `
		'${CMAKE_SOURCE_DIR}/readme.txt' `
		-DestinationPath '${ZIP_FILE}'"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	COMMENT "Creating ZIP archive..."
	VERBATIM
)

add_dependencies(package client server)
