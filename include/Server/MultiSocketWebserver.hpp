#pragma once

#include <sys/poll.h>

#include <vector>

#include "PollFdManager.hpp"
#include "ServerConfig.hpp"

class MultiSocketWebserver {
	private:
		std::vector<ServerConfig> _servers_config;
		std::vector<Socket> _sockets;
		PollFdManager& _polls;

		void _acceptConnection(int server_fd);
		void _handleClientData(int client_fd);
		bool _isServerSocket(int fd);

	public:
		explicit MultiSocketWebserver(std::vector<ServerConfig> servers_config);
		~MultiSocketWebserver();

		void run();
		void initSockets();
};
