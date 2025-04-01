#pragma once

#include <windows.h>

constexpr UINT ID_TRAY_ICON = 101;
constexpr UINT ID_TRAY_CALLBACK = WM_USER + 1;

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
