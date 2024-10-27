
#include "Socket.hpp"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "Logger.hpp"

Socket::Socket(int port) : _port(port) {
	LOG_INFO("Creating socket");
	// Create socket
	_socketFd = socket(AF_INET, SOCK_STREAM, 0);

	if (_socketFd == -1)
		throw std::runtime_error("Socket creation failed");

	// Set socket options
	int opt = 1;
	if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw std::runtime_error("Setsockopt failed");
	}

	// Set address
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET; // IPv4
	_addr.sin_addr.s_addr = INADDR_ANY; // allowing to bind to any available network interface
	_addr.sin_port = htons(port); // Convert to network byte order
}

Socket::~Socket() {
	close(_socketFd);
}

void Socket::bind() {
	LOG_DEBUG("Binding socket to port " + std::to_string(_port));
	if (::bind(_socketFd, reinterpret_cast<sockaddr *>(&_addr), sizeof(_addr)) == -1) {
		throw std::runtime_error("Bind failed");
	}
}

void Socket::listen() const {
	LOG_DEBUG("Listening on socket");
	if (::listen(_socketFd, SOMAXCONN) == -1) {
		throw std::runtime_error("Listen failed");
	}
}

int Socket::accept() {
	LOG_DEBUG("Accepting connection");

	socklen_t addrLen = sizeof(_addr);
	const int clientFd = ::accept(_socketFd, reinterpret_cast<struct sockaddr *>(&_addr), &addrLen);
	if (clientFd == -1) {
		throw std::runtime_error("Accept failed");
	}
	return clientFd;
}
