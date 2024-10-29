#pragma once

#include <sys/poll.h>

#include <unordered_map>
#include <vector>

#include "PollFdManager.hpp"
#include "ServerConfig.hpp"

class MultiSocketWebserver {
	private:
		std::vector<ServerConfig> _servers_config;
		std::vector<Socket> _sockets;  // TODO: change to unordered_map unique_ptr
		std::unordered_map<int, std::unique_ptr<ClientConnection>> _clients;
		PollFdManager& _polls;

		void _acceptConnection(int server_fd);
		void _handleClientData(int client_fd);

	public:
		explicit MultiSocketWebserver(std::vector<ServerConfig> servers_config);
		~MultiSocketWebserver();

		void run();
		void initSockets();
};
