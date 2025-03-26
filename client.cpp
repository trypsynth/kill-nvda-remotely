#define WIN32_LEAN_AND_MEAN
#include <filesystem>
#include <iostream>
#include <vector>
#include <shlwapi.h>
#include <windows.h>
#include "client.hpp"

namespace fs = std::filesystem;

winsock::winsock() {
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		MessageBox(nullptr, "Failed to initialize Winsock", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
}

winsock::~winsock() {
	WSACleanup();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
	try {
		fs::path configPath = fs::current_path() / "config.ini";
		auto host = get_config_value(configPath.string(), "Settings", "Host");
		auto portStr = get_config_value(configPath.string(), "Settings", "Port");
		auto packet = get_config_value(configPath.string(), "Settings", "Packet").value_or("kill");
		if (!host || !portStr) {
			MessageBox(nullptr, "Invalid or missing configuration file", "Error", MB_OK | MB_ICONERROR);
			return 1;
		}
		int port = std::stoi(portStr.value());
		winsock winsock;
		SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (client_socket == INVALID_SOCKET) {
			MessageBox(nullptr, "Failed to create socket", "Error", MB_OK | MB_ICONERROR);
			return 1;
		}
		sockaddr_in server_addr{};
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(static_cast<u_short>(port));
		server_addr.sin_addr.s_addr = inet_addr(host->c_str());
		if (server_addr.sin_addr.s_addr == INADDR_NONE) {
			MessageBox(nullptr, "Invalid server address", "Error", MB_OK | MB_ICONERROR);
			closesocket(client_socket);
			return 1;
		}
		if (connect(client_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
			MessageBox(nullptr, "Failed to connect to the server", "Error", MB_OK | MB_ICONERROR);
			closesocket(client_socket);
			return 1;
		}
		if (send(client_socket, packet.c_str(), packet.length(), 0) == SOCKET_ERROR) MessageBox(nullptr, "Failed to send data", "Error", MB_OK | MB_ICONERROR);
		closesocket(client_socket);
		return 0;
	} catch (const std::exception& e) {
		MessageBox(nullptr, e.what(), "Exception", MB_OK | MB_ICONERROR);
		return 1;
	}
}

std::optional<std::string> get_config_value(const std::string& filePath, const std::string& section, const std::string& key) {
	char buffer[256]{0};
	if (GetPrivateProfileString(section.c_str(), key.c_str(), nullptr, buffer, sizeof(buffer), filePath.c_str()) == 0) return std::nullopt;
	return std::string(buffer);
}
