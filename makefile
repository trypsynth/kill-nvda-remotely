CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -Os -ffunction-sections -fdata-sections -fno-rtti
LDFLAGS = -lws2_32 -lshlwapi -Wl,--gc-sections -Wl,--subsystem,windows -s
OUTPUT_DIR = .
ZIP_FILE = $(OUTPUT_DIR)/kill_nvda_remotely.zip
SOURCES_CLIENT = client.cpp
SOURCES_SERVER = server.cpp
OBJ_CLIENT = $(SOURCES_CLIENT:.cpp=.o)
OBJ_SERVER = $(SOURCES_SERVER:.cpp=.o)

all: client.exe server.exe package

client.exe: $(OBJ_CLIENT)
	$(CXX) -o $@ $^ $(LDFLAGS)

server.exe: $(OBJ_SERVER)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

package: client.exe server.exe
	@echo "Packaging binaries into $(ZIP_FILE)"
	powershell -Command "Compress-Archive -Force -Path 'client.exe', 'server.exe', 'config.ini', 'readme.txt' -DestinationPath '$(ZIP_FILE)'"

clean:
	rm -f client.exe server.exe $(OBJ_CLIENT) $(OBJ_SERVER) $(ZIP_FILE)

.PHONY: all package clean
