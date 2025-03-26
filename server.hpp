#pragma once

#include <cstdint>
#include <string>
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
	explicit server_socket(uint16_t port);
	~server_socket();
	SOCKET accept_connection() const;

private:
	SOCKET handle_;
};

class single_instance {
public:
	explicit single_instance(const std::string& app_id);
	~single_instance();

private:
	HANDLE handle_;
};

class tray_icon {
public:
	tray_icon(HWND window);
	~tray_icon();

private:
	NOTIFYICONDATA nid_{};
};

class winsock {
public:
	winsock();
	~winsock();
};
