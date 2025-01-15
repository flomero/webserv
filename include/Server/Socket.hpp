
#pragma once

#include <netinet/in.h>

#include "ServerConfig.hpp"

class Socket {
	public:
		explicit Socket(std::vector<ServerConfig> configs);
		~Socket();

		void bind();
		void listen() const;
		[[nodiscard]] int getSocketFd() const;
		[[nodiscard]] std::vector<ServerConfig> getConfig() const;

	private:
		void setupAddress();
		void setSocketOpt() const;
		int _socketFd;
		int _port;
		ServerConfig& _default_config;
		std::vector<ServerConfig> _configs;
		sockaddr_in _addr;
};
