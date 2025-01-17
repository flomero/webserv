#include "Socket.hpp"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "Logger.hpp"
#include "ServerConfig.hpp"

Socket::Socket(std::vector<ServerConfig> configs)
	: _socketFd(-1),
	  _port(configs.front().getPort()),
	  _default_config(configs.front()),
	  _configs(std::move(configs)),
	  _addr(sockaddr_in{}) {
	LOG_INFO("Creating socket on IP " + _default_config.getHostIP() + " and port " + std::to_string(_port));
	_socketFd = socket(AF_INET, SOCK_STREAM, 0);

	if (_socketFd == -1)
		throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));

	setSocketOpt();
	setupAddress();
	bind();
	listen();
}

Socket::~Socket() {
	LOG_INFO("Closing socket");
	if (_socketFd != -1) {
		close(_socketFd);
	}
}

void Socket::bind() {
	LOG_DEBUG("Binding socket to IP " + _default_config.getHostIP() + " and port " + std::to_string(_port));
	if (::bind(_socketFd, reinterpret_cast<sockaddr *>(&_addr), sizeof(_addr)) == -1) {
		throw std::runtime_error("Bind failed on IP " + _default_config.getHostIP() + " and port " +
								 std::to_string(_port) + ": " + std::string(strerror(errno)));
	}
	LOG_INFO("Socket successfully bound to IP " + _default_config.getHostIP() + " and port " + std::to_string(_port));
}

void Socket::listen() const {
	LOG_DEBUG("Listening on socket");
	if (::listen(_socketFd, SOMAXCONN) == -1) {
		throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
	}
}

int Socket::getSocketFd() const { return _socketFd; }

void Socket::setupAddress() {
	_addr = sockaddr_in{};												// Value-initialize sockaddr_in
	_addr.sin_family = AF_INET;											// IPv4
	_addr.sin_addr.s_addr = my_inet_addr(_default_config.getHostIP());	// Convert IP address to network byte order
	_addr.sin_port = htons(_port);										// Convert port to network byte order
}

void Socket::setSocketOpt() const {
	// Set socket options
	constexpr int opt = 1;
	if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw std::runtime_error("Setsockopt failed: " + std::string(strerror(errno)));
	}
}

std::vector<ServerConfig> Socket::getConfig() const { return _configs; }

uint32_t my_inet_addr(const std::string &ipStr) {
	uint32_t result = 0;
	int partsCount = 0;

	std::stringstream ss(ipStr);
	std::string part;

	// Split the input string by '.'
	while (std::getline(ss, part, '.')) {
		if (partsCount >= 4)
			throw std::runtime_error("Invalid IPv4 address format: " + ipStr);
		// Convert the current part to an integer [0..255]
		int value = 0;
		try {
			value = std::stoi(part);
		} catch (...) {
			throw std::runtime_error("Invalid integer in IP address: " + part);
		}
		if (value < 0 || value > 255)
			throw std::runtime_error("IP octet out of range (0-255): " + part);

		// Shift the current result by 8 bits and add the new value
		result = (result << 8) | static_cast<uint32_t>(value & 0xFF);
		++partsCount;
	}

	// Make sure we had exactly 4 parts
	if (partsCount != 4) {
		throw std::runtime_error("Invalid IPv4 address format: " + ipStr);
	}

	return result;
}

std::string my_inet_ntoa(const in_addr &in) {
	// The address is stored in network byte order (big-endian).
	// For example, 192.168.0.1 -> 0xC0A80001 in memory.

	uint32_t ip = in.s_addr;  // 32-bit IP in network byte order

	// Extract each octet by shifting and masking
	unsigned char b1 = (ip >> 24) & 0xFF;
	unsigned char b2 = (ip >> 16) & 0xFF;
	unsigned char b3 = (ip >> 8) & 0xFF;
	unsigned char b4 = ip & 0xFF;

	// Build the dot-decimal string
	std::ostringstream oss;
	oss << static_cast<int>(b1) << "." << static_cast<int>(b2) << "." << static_cast<int>(b3) << "."
		<< static_cast<int>(b4);

	return oss.str();
}
