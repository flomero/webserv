#include "MultiSocketWebserver.hpp"

#include <arpa/inet.h>

#include "PollFdManager.hpp"

MultiSocketWebserver::MultiSocketWebserver(std::vector<ServerConfig> servers_config)
	: _polls(PollFdManager::getInstance()) {
	_servers_config = std::move(servers_config);
}

void MultiSocketWebserver::initSockets() {
	_sockets.reserve(_servers_config.size());
	for (const auto& serv : _servers_config) {
		_sockets.emplace_back(serv.getPort());
		const int server_fd = _sockets.back().getSocketFd();
		_polls.addFd(server_fd);
	}
}

MultiSocketWebserver::~MultiSocketWebserver() {	 // TODO: Implement destructor
	for (const auto& socket : _sockets) {
		close(socket.getSocketFd());
	}
}

void MultiSocketWebserver::run() {
	while (true) {
		if (const int eventCount = poll(_polls.data(), _polls.size(), -1); eventCount == -1) {
			LOG_ERROR("Poll failed: " + std::string(strerror(errno)));
			break;
		}

		for (auto& [fd, events, revents] : _polls.getPolls()) {
			if (revents & POLLIN) {
				if (_isServerSocket(fd)) {
					_acceptConnection(fd);
				} else {
					_handleClientData(fd);
				}
			}
		}
	}

	// TODO: Poll failed, handle error
}

void MultiSocketWebserver::_acceptConnection(const int server_fd) {
	sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	const int clientFd = ::accept(server_fd, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);

	if (clientFd == -1) {
		LOG_ERROR("Accept failed: " + std::string(strerror(errno)));
		return;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
		LOG_ERROR("Failed to set client socket to non-blocking: " + std::string(strerror(errno)));
		close(clientFd);
		return;
	}

	_polls.addFd(clientFd);
	LOG_INFO("Accepted connection from " + std::string(inet_ntoa(clientAddr.sin_addr)) + " on socket " +
			 std::to_string(clientFd));
}

void MultiSocketWebserver::_handleClientData(const int client_fd) {
	char buffer[4096];
	ssize_t bytesReceived = recv(client_fd, buffer, sizeof(buffer), 0);
	if (bytesReceived <= 0) {
		if (bytesReceived == 0 || (bytesReceived == -1 && errno != EAGAIN)) {
			close(client_fd);
			_polls.removeFd(client_fd);
		}
		return;
	}

	std::string request(buffer, bytesReceived);
	LOG_INFO("Received request: " + request);

	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	send(client_fd, response.c_str(), response.size(), 0);

	close(client_fd);
	_polls.removeFd(client_fd);
}

bool MultiSocketWebserver::_isServerSocket(int fd) {
	return std::find_if(_sockets.begin(), _sockets.end(), [fd](const Socket& s) { return s.getSocketFd() == fd; }) !=
		   _sockets.end();
}
