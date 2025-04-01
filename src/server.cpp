#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <windows.h>
#include <shellapi.h>
#include "server_socket.hpp"
#include "single_instance.hpp"
#include "tray_icon.hpp"
#include "winsock.hpp"

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
std::string get_config_value(const std::string& file, const std::string& section, const std::string& key, const std::string& default_value);
std::string get_config_file();

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, PSTR, int) {
	try {
		single_instance inst("KillNVDARemotely_Server");
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = window_proc;
		wc.hInstance = instance;
		wc.lpszClassName = "KillNVDARemotely_Server";
		if (!RegisterClassEx(&wc)) throw std::runtime_error{"Failed to register window class"};
		HWND window = CreateWindowEx(0, wc.lpszClassName, "KillNVDARemotely_Server", 0, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
		if (!window) throw std::runtime_error{"Failed to create window"};
		tray_icon tray_icon(window);
		std::string config_file = get_config_file();
		std::string port_str = get_config_value(config_file, "Settings", "Port", "");
		std::string packet = get_config_value(config_file, "Settings", "Packet", "kill");
		if (port_str.empty()) throw std::runtime_error{"Invalid or missing configuration file"};
		uint16_t port = static_cast<uint16_t>(std::stoi(port_str));
		winsock winsock;
		server_socket server(port);
		MSG msg;
		while (true) {
			Sleep(5);
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) return 0;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			fd_set read_set;
			FD_ZERO(&read_set);
			FD_SET(server.accept_connection(), &read_set);
			timeval timeout{};
			if (select(0, &read_set, nullptr, nullptr, &timeout) > 0) {
				SOCKET client_socket = server.accept_connection();
				if (client_socket == INVALID_SOCKET) throw std::runtime_error{"Accept failed"};
				char buffer[1024];
				int bytes_received;
				while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
					buffer[bytes_received] = '\0';
					if (packet == buffer) system("start nvda -r");
				}
				closesocket(client_socket);
			}
		}
	} catch (const std::exception& e) {
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
		case ID_TRAY_CALLBACK:
			if (lp == WM_RBUTTONUP || lp == WM_LBUTTONUP) {
				POINT cursor_pos;
				GetCursorPos(&cursor_pos);
				HMENU menu = CreatePopupMenu();
				AppendMenu(menu, MF_STRING, 1, "Exit");
				SetForegroundWindow(hwnd);
				int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, cursor_pos.x, cursor_pos.y, 0, hwnd, nullptr);
				DestroyMenu(menu);
				if (cmd == 1) PostQuitMessage(0);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

std::string get_config_value(const std::string& file, const std::string& section, const std::string& key, const std::string& default_value) {
	char buffer[256];
	if (GetPrivateProfileString(section.c_str(), key.c_str(), default_value.c_str(), buffer, sizeof(buffer), file.c_str()) == 0) return default_value;
	return buffer;
}

std::string get_config_file() {
	std::filesystem::path config_path = std::filesystem::current_path() / "config.ini";
	if (!std::filesystem::exists(config_path)) throw std::runtime_error{"Configuration file not found: " + config_path.string()};
	return config_path.string();
}
