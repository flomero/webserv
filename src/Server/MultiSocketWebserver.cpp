#include "MultiSocketWebserver.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include "ClientConnection.hpp"
#include "Logger.hpp"
#include "PollFdManager.hpp"
#include "Socket.hpp"

MultiSocketWebserver::MultiSocketWebserver(std::vector<ServerConfig> servers_config)
	: _polls(PollFdManager::getInstance()) {
	_servers_config = std::move(servers_config);
}

void MultiSocketWebserver::initSockets() {
	_sockets.reserve(_servers_config.size());
	for (ServerConfig& serv : _servers_config) {
		try {
			auto newSocket = std::make_unique<Socket>(serv.getPort(), serv);
			int socketFd = newSocket->getSocketFd();
			_sockets.emplace(socketFd, std::move(newSocket));
			_polls.addFd(socketFd);
		} catch (const std::exception& e) {
			LOG_ERROR("Failed to create socket: " + std::string(e.what()));
		}
	}

	if (_sockets.empty()) {
		throw std::runtime_error("Failed to create any sockets");
	}
}

MultiSocketWebserver::~MultiSocketWebserver() {	 // TODO: Implement destructor
	for (const auto& [fd, _] : _sockets) {
		close(fd);
	}
}

void MultiSocketWebserver::run() {
	while (true) {
		// TODO timeout
		if (const int eventCount = poll(_polls.data(), _polls.size(), 5000); eventCount == -1) {
			LOG_ERROR("Poll failed: " + std::string(strerror(errno)));
			break;
		}

		for (auto& [fd, events, revents] : _polls.getPolls()) {
			// Check if there are any events to process
			if (revents & POLLIN) {
				if (isServerFd(fd)) {
					_acceptConnection(fd);
					break;
				}
				_handleClientData(fd);
				break;
			}
			if (revents & POLLOUT) {
				// LOG_INFO("Write event on socket " + std::to_string(fd));
				_handleClientWrite(fd);
				break;
			}
			if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
				LOG_ERROR("Error on socket " + std::to_string(fd));
				_polls.removeFd(fd);
			}
		}
	}

	// TODO: Poll failed, handle error
}

void MultiSocketWebserver::_acceptConnection(const int server_fd) {
	sockaddr_in clientAddr{};
	socklen_t addrLen = sizeof(clientAddr);
	int clientFd = accept(server_fd, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);

	if (clientFd == -1) {
		LOG_ERROR("Accept failed: " + std::string(strerror(errno)));
		return;
	}

	timeval tv{};
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	try {
		_clients.emplace(clientFd,
						 std::make_unique<ClientConnection>(clientFd, clientAddr, _sockets.at(server_fd)->getConfig()));
		_polls.addFd(clientFd);
		LOG_INFO("Accepted connection from " + std::string(inet_ntoa(clientAddr.sin_addr)) + " on socket " +
				 std::to_string(clientFd));
	} catch (const std::exception& e) {
		LOG_ERROR("Failed to create ClientConnection: " + std::string(e.what()));
		close(clientFd);
	}
}

void MultiSocketWebserver::_handleClientData(const int client_fd) {
	auto it = _clients.find(client_fd);
	if (it == _clients.end()) {
		LOG_ERROR("Client not found in map");
		return;
	}

	ClientConnection& client = *it->second;
	client.handleClient();

	if (client.isDisconnected()) {
		_clients.erase(client_fd);
		_polls.removeFd(client_fd);
		LOG_DEBUG("Client disconnected from socket " + std::to_string(client_fd) + " after read");
	}
}

bool MultiSocketWebserver::isServerFd(int fd) const { return _sockets.find(fd) != _sockets.end(); }

void MultiSocketWebserver::_handleClientWrite(int fd) {
	auto it = _clients.find(fd);
	if (it == _clients.end()) {
		LOG_ERROR("Client not found in map");
		return;
	}

	ClientConnection& client = *it->second;
	client.sendResponse();

	if (client.isDisconnected()) {
		_clients.erase(fd);
		_polls.removeFd(fd);
		LOG_DEBUG("Client disconnected from socket " + std::to_string(fd) + " after write");
	}
}
