
#pragma once

#include <netinet/in.h>

#include "ServerConfig.hpp"

class Socket {
	public:
		explicit Socket(int port, ServerConfig& config);
		~Socket();

		void bind();
		void listen() const;
		[[nodiscard]] int getSocketFd() const;
		[[nodiscard]] ServerConfig& getConfig() const;

	private:
		void setupAddress();
		void setSocketOpt() const;
		int _socketFd;
		int _port;
		ServerConfig& _config;
		sockaddr_in _addr;
};
