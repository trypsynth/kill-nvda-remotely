#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <winsock2.h>

constexpr UINT ID_TRAY_ICON = 101;
constexpr UINT ID_TRAY_CALLBACK = WM_USER + 1;

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
std::string get_config_value(const std::string& file, const std::string& section, const std::string& key, const std::string& default_value);
std::string get_config_file();

class server_socket {
public:
	explicit server_socket(uint16_t port) {
		handle_ = socket(AF_INET, SOCK_STREAM, 0);
		if (handle_ == INVALID_SOCKET) throw std::runtime_error{"Failed to create socket"};
		sockaddr_in server_address{};
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(port);
		server_address.sin_addr.s_addr = INADDR_ANY;
		if (bind(handle_, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
			closesocket(handle_);
			throw std::runtime_error{"Bind failed"};
		}
		if (listen(handle_, SOMAXCONN) == SOCKET_ERROR) {
			closesocket(handle_);
			throw std::runtime_error{"Listen failed"};
		}
	}

	~server_socket() {
		closesocket(handle_);
	}

	SOCKET accept_connection() const {
		return accept(handle_, nullptr, nullptr);
	}

private:
	SOCKET handle_;
};

class single_instance {
public:
	explicit single_instance(const std::string& app_id) {
		std::string mutex_name = app_id + "_IsAlreadyRunning";
		handle_ = CreateMutex(nullptr, TRUE, mutex_name.c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS) throw std::runtime_error{"Another instance is already running"};
	}

	~single_instance() {
		if (handle_) CloseHandle(handle_);
	}

private:
	HANDLE handle_;
};

class tray_icon {
public:
	explicit tray_icon(HWND window) {
		nid_.cbSize = sizeof(NOTIFYICONDATA);
		nid_.hWnd = window;
		nid_.uID = ID_TRAY_ICON;
		nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid_.uCallbackMessage = ID_TRAY_CALLBACK;
		nid_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		strcpy_s(nid_.szTip, "Kill NVDA Remotely Server");
		if (!Shell_NotifyIcon(NIM_ADD, &nid_)) throw std::runtime_error{"Failed to create tray icon"};
	}

	~tray_icon() {
		Shell_NotifyIcon(NIM_DELETE, &nid_);
	}

private:
	NOTIFYICONDATA nid_{};
};

class winsock {
public:
	winsock() {
		WSADATA wsa_data;
		if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) throw std::runtime_error{"WSAStartup failed"};
	}

	~winsock() {
		WSACleanup();
	}
};

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
