#pragma once

#include <cstring>	// For strerror
#include <unordered_map>
#include <vector>

#include "PollFdManager.hpp"
#include "ServerConfig.hpp"

class ClientConnection;
class Socket;

class MultiSocketWebserver {
		std::vector<std::vector<ServerConfig>> _server_configs_vector;
		std::unordered_map<int, std::unique_ptr<Socket>> _sockets;
		std::unordered_map<int, std::unique_ptr<ClientConnection>> _clients;
		PollFdManager& _polls;

		void _acceptConnection(int server_fd);
		bool _handleClientData(int client_fd);
		[[nodiscard]] bool isServerFd(int fd) const;

	public:
		explicit MultiSocketWebserver(std::vector<std::vector<ServerConfig>> servers_config);

		~MultiSocketWebserver();

		bool _handleClientWrite(int fd);
		void run();
		void initSockets();
};
