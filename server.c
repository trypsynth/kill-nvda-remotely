#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <shellapi.h>
#include <shlwapi.h>

#define ID_TRAY_ICON 100
#define ID_TRAY_CALLBACK WM_USER + 1

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MakeSingleInstance(const char* AppID);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	MakeSingleInstance("KillNVDARemotely_Server");
	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "KillNVDARemotely_Server";
	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	HWND hWnd = CreateWindowEx(0, wc.lpszClassName, "KillNVDARemotely_Server", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		MessageBox(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = ID_TRAY_ICON;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = ID_TRAY_CALLBACK;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	strncpy(nid.szTip, "Kill NVDA Remotely Server", sizeof(nid.szTip));
	if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
		MessageBox(NULL, "Failed to create tray icon", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	char szPort[16] = {0};
	char szPacket[32] = {0};
	char szConfigFile[MAX_PATH] = {0};
	if (GetCurrentDirectory(sizeof(szConfigFile), szConfigFile) == 0) {
		MessageBox(NULL, "Failed to get the current working directory", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	if (!PathCombine(szConfigFile, szConfigFile, "config.ini")) {
		MessageBox(NULL, "Failed to construct the config file path", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	if (GetPrivateProfileString("Settings", "Port", NULL, szPort, sizeof(szPort), szConfigFile) == 0) {
		MessageBox(NULL, "Invalid or missing configuration file", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	if (GetPrivateProfileString("Settings", "Packet", NULL, szPacket, sizeof(szPacket), szConfigFile) == 0) {
		strncpy(szPacket, "kill", sizeof(szPacket));
	}
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		MessageBox(NULL, "WSAStartup failed", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	SOCKET hServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET) {
		MessageBox(NULL, "Failed to create socket", "Error", MB_OK | MB_ICONERROR);
		WSACleanup();
		ExitProcess(1);
	}
	struct sockaddr_in stServerAddress = {0};
	stServerAddress.sin_family = AF_INET;
	stServerAddress.sin_port = htons(atoi(szPort));
	stServerAddress.sin_addr.s_addr = INADDR_ANY;
	if (bind(hServerSocket, (struct sockaddr*)&stServerAddress, sizeof(stServerAddress)) == SOCKET_ERROR) {
		MessageBox(NULL, "Bind failed", "Error", MB_OK | MB_ICONERROR);
		closesocket(hServerSocket);
		WSACleanup();
		ExitProcess(1);
	}
	if (listen(hServerSocket, SOMAXCONN) == SOCKET_ERROR) {
		MessageBox(NULL, "Listen failed", "Error", MB_OK | MB_ICONERROR);
		closesocket(hServerSocket);
		WSACleanup();
		ExitProcess(1);
	}
	MSG msg;
	while (1) {
		Sleep(5);
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				closesocket(hServerSocket);
				WSACleanup();
				Shell_NotifyIcon(NIM_DELETE, &nid);
				ExitProcess(0);
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(hServerSocket, &readSet);
		struct timeval timeout = {0, 0}; // Non-blocking.
		if (select(0, &readSet, NULL, NULL, &timeout) > 0) {
			SOCKET hClientSocket = accept(hServerSocket, NULL, NULL);
			if (hClientSocket == INVALID_SOCKET) {
				MessageBox(NULL, "Accept failed", "Error", MB_OK | MB_ICONERROR);
				closesocket(hServerSocket);
				WSACleanup();
				ExitProcess(1);
			}
			char szBuffer[1024];
			int nBytesReceived;
			while ((nBytesReceived = recv(hClientSocket, szBuffer, sizeof(szBuffer) - 1, 0)) > 0) {
				szBuffer[nBytesReceived] = '\0';
				if (strcmp(szBuffer, szPacket) == 0) system("start nvda -r");
			}
			closesocket(hClientSocket);
		}
	}
	closesocket(hServerSocket);
	WSACleanup();
	Shell_NotifyIcon(NIM_DELETE, &nid);
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case ID_TRAY_CALLBACK:
			PostQuitMessage(0);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void MakeSingleInstance(const char* AppID) {
	char mutexName[128];
	snprintf(mutexName, sizeof(mutexName), "%s_IsAlreadyRunning", AppID);
	HANDLE hMutex = CreateMutex(NULL, TRUE, mutexName);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, "Another instance is already running", "Error", MB_ICONERROR | MB_OK);
		ExitProcess(0);
	}
}
