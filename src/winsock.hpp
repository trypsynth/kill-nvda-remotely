#pragma once

#include <stdexcept>

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
