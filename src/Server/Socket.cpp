#include "Socket.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "Logger.hpp"
#include "ServerConfig.hpp"

Socket::Socket(const int port, ServerConfig& config)
	: _socketFd(-1), _port(port), _config(config), _addr(sockaddr_in{}) {
	LOG_INFO("Creating socket on port " + std::to_string(port));
	_socketFd = socket(AF_INET, SOCK_STREAM, 0);

	if (_socketFd == -1)
		throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));

	setSocketOpt();
	setupAddress();
	bind();
	listen();
}

Socket::~Socket() {
	if (_socketFd != -1) {
		close(_socketFd);
	}
}

void Socket::bind() {
	LOG_DEBUG("Binding socket to port " + std::to_string(_port));
	if (::bind(_socketFd, reinterpret_cast<sockaddr*>(&_addr), sizeof(_addr)) == -1) {
		throw std::runtime_error("Bind failed on port " + std::to_string(_port) + ": " + std::string(strerror(errno)));
	}
	LOG_INFO("Socket successfully bound to port " + std::to_string(_port));
}

void Socket::listen() const {
	LOG_DEBUG("Listening on socket");
	if (::listen(_socketFd, SOMAXCONN) == -1) {
		throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
	}
}

int Socket::getSocketFd() const { return _socketFd; }

void Socket::setupAddress() {
	_addr = sockaddr_in{};				 // Value-initialize sockaddr_in
	_addr.sin_family = AF_INET;			 // IPv4
	_addr.sin_addr.s_addr = INADDR_ANY;	 // Bind to any available network interface
	_addr.sin_port = htons(_port);		 // Convert port to network byte order
}

void Socket::setSocketOpt() const {
	// Set socket options
	constexpr int opt = 1;
	if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw std::runtime_error("Setsockopt failed: " + std::string(strerror(errno)));
	}
}

ServerConfig& Socket::getConfig() const { return _config; }
