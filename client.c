#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <shlwapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	char szHost[256] = {0};;
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
	if (GetPrivateProfileString("Settings", "Host", NULL, szHost, sizeof(szHost), szConfigFile) == 0 || GetPrivateProfileString("Settings", "Port", NULL, szPort, sizeof(szPort), szConfigFile) == 0) {
		MessageBox(NULL, "Invalid or missing configuration file", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	if (GetPrivateProfileString("Settings", "Packet", NULL, szPacket, sizeof(szPacket), szConfigFile) == 0) {
		strncpy(szPacket, "kill", sizeof(szPacket));
	}
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		MessageBox(NULL, "Failed to initialize Winsock", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}
	SOCKET hClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hClientSocket == INVALID_SOCKET) {
		MessageBox(NULL, "Failed to create socket", "Error", MB_OK | MB_ICONERROR);
		WSACleanup();
		ExitProcess(1);
	}
	struct sockaddr_in stServerAddress = {0};
	stServerAddress.sin_family = AF_INET;
	stServerAddress.sin_port = htons((u_short)atoi(szPort));
	stServerAddress.sin_addr.s_addr = inet_addr(szHost);
	if (stServerAddress.sin_addr.s_addr == INADDR_NONE) {
		MessageBox(NULL, "Invalid server address", "Error", MB_OK | MB_ICONERROR);
		closesocket(hClientSocket);
		WSACleanup();
		ExitProcess(1);
	}
	if (connect(hClientSocket, (struct sockaddr*)&stServerAddress, sizeof(stServerAddress)) == SOCKET_ERROR) {
		MessageBox(NULL, "Failed to connect to the server", "Error", MB_OK | MB_ICONERROR);
		closesocket(hClientSocket);
		WSACleanup();
		ExitProcess(1);
	}
	if (send(hClientSocket, szPacket, strlen(szPacket), 0) == SOCKET_ERROR) {
		MessageBox(NULL, "Failed to send data", "Error", MB_OK | MB_ICONERROR);
	}
	closesocket(hClientSocket);
	WSACleanup();
	ExitProcess(0);
}
