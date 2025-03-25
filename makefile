CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lws2_32 -lshlwapi
OUTPUT_DIR = .
ZIP_FILE = $(OUTPUT_DIR)/kill_nvda_remotely.zip
SOURCES_CLIENT = client.c
SOURCES_SERVER = server.c
OBJ_CLIENT = $(SOURCES_CLIENT:.c=.o)
OBJ_SERVER = $(SOURCES_SERVER:.c=.o)

all: client.exe server.exe package

client.exe: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(LDFLAGS)

server.exe: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

package: client.exe server.exe
	@echo "Packaging binaries into $(ZIP_FILE)"
	powershell -Command "Compress-Archive -Force -Path 'client.exe', 'server.exe', 'config.ini', 'readme.txt' -DestinationPath '$(ZIP_FILE)'"

clean:
	rm -f client.exe server.exe $(OBJ_CLIENT) $(OBJ_SERVER) $(ZIP_FILE)

.PHONY: all package clean
