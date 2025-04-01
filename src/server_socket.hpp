#pragma once

#include <cstdint>
#include <stdexcept>

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
