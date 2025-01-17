#include "MultiSocketWebserver.hpp"

#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/sysctl.h>
#include <unistd.h>

#include <cstddef>
#include <random>

#include "ClientConnection.hpp"
#include "Logger.hpp"
#include "PollFdManager.hpp"
#include "Socket.hpp"
#include "globals.hpp"

MultiSocketWebserver::MultiSocketWebserver(std::vector<std::vector<ServerConfig>> servers_config)
	: _polls(PollFdManager::getInstance()) {
	_server_configs_vector = std::move(servers_config);
}

void MultiSocketWebserver::initSockets() {
	_sockets.reserve(_server_configs_vector.size());
	for (const std::vector<ServerConfig>& serv : _server_configs_vector) {
		try {
			auto newSocket = std::make_unique<Socket>(serv);
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

MultiSocketWebserver::~MultiSocketWebserver() {
	for (const auto& [fd, _] : _sockets) {
		if (fd != -1) {
			close(fd);
		}
	}
	_sockets.clear();

	for (const auto& [fd, _] : _clients) {
		if (fd != -1) {
			close(fd);
		}
	}
	_clients.clear();
}

void MultiSocketWebserver::run() {
	while (stopServer == false) {
		if (const int eventCount = poll(_polls.data(), _polls.size(), 5000); eventCount == -1 && !stopServer) {
			LOG_ERROR("Poll failed: " + std::string(strerror(errno)));
			break;
		}

		for (auto& [fd, events, revents] : _polls.getPolls()) {
			if (stopServer) {
				break;
			}
			// Check if there are any events to process
			if (revents & POLLIN) {
				if (isServerFd(fd)) {
					_acceptConnection(fd);
				} else {
					_handleClientData(fd);
				}
			}
			if (revents & POLLOUT) {
				_handleClientWrite(fd);
			}
			if (revents & (POLLERR | POLLHUP | POLLNVAL | POLLPRI)) {
				if (revents & POLLHUP) {
					LOG_INFO("Client disconnected from socket " + std::to_string(fd));
				} else {
					LOG_ERROR("Error on socket " + std::to_string(fd));
				}
				if (isServerFd(fd)) {
					_sockets.erase(fd);
				} else {
					_clients.erase(fd);
				}
				if (fd != -1) {
					close(fd);
				}
				_polls.removeFd(fd);
			}
		}
	}
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

bool MultiSocketWebserver::_handleClientData(const int client_fd) {
	auto it = _clients.find(client_fd);
	if (it == _clients.end()) {
		LOG_ERROR("Client not found in map");
		return false;
	}

	ClientConnection& client = *it->second;
	if (client.getStatus() == ClientConnection::Status::READY_TO_SEND) {
		return false;
	}
	client.handleClient();

	if (client.isDisconnected()) {
		_clients.erase(client_fd);
		_polls.removeFd(client_fd);
		LOG_DEBUG("Client disconnected from socket " + std::to_string(client_fd) + " after read");
		return true;
	}

	return true;
}

bool MultiSocketWebserver::isServerFd(int fd) const { return _sockets.find(fd) != _sockets.end(); }

bool MultiSocketWebserver::_handleClientWrite(int fd) {
	auto it = _clients.find(fd);
	if (it == _clients.end()) {
		LOG_ERROR("Client not found in map");
		return false;
	}

	ClientConnection& client = *it->second;
	if (client.getStatus() != ClientConnection::Status::READY_TO_SEND &&
		client.getStatus() != ClientConnection::Status::SENDING_RESPONSE) {
		return false;
	}

	client.sendResponse();

	if (client.isDisconnected()) {
		_clients.erase(fd);
		_polls.removeFd(fd);
		LOG_DEBUG("Client disconnected from socket " + std::to_string(fd) + " after write");
		return false;
	}

	return true;
}
