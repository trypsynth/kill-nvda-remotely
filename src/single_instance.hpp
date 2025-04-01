#pragma once

#include <stdexcept>
#include <string>
#include <windows.h>

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
