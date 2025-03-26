#pragma once

#include <optional>
#include <string>
#include <winsock2.h>

std::optional<std::string> get_config_value(const std::string& file_path, const std::string& section, const std::string& key);

class winsock {
public:
	winsock();
	~winsock();

private:
	WSADATA wsa_data;
};
