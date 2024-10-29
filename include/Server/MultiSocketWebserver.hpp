#pragma once

#include <sys/poll.h>

#include <vector>

#include "ServerConfig.hpp"

class MultiSocketWebserver {
	private:
		std::vector<pollfd> _pollFds;
		std::vector<ServerConfig> _servers_config;
		std::vector<Socket> _sockets;

		void _acceptConnection(int server_fd);
		void _handleClientData(int client_fd);
		void _deleteFd(int fd);
		bool _isServerSocket(int fd);

	public:
		explicit MultiSocketWebserver(std::vector<ServerConfig> servers_config);
		~MultiSocketWebserver();

		void run();
		void initSockets();
};
