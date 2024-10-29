
#include "ClientConnection.hpp"

#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"

ClientConnection::ClientConnection(const int clientFd, const sockaddr_in clientAddr)
	: _clientFd(clientFd), _disconnected(false), _clientAddr(clientAddr) {
	if (fcntl(_clientFd, F_SETFL, O_NONBLOCK) == -1) {
		LOG_ERROR("Failed to set client socket to non-blocking: " + std::string(strerror(errno)));
		_disconnected = true;
		close(_clientFd);
	}
	_requestBuffer.reserve(1024);
	LOG_INFO("Address: " + std::string(inet_ntoa(_clientAddr.sin_addr)) +
			 " Port: " + std::to_string(ntohs(_clientAddr.sin_port)));
}

ClientConnection::~ClientConnection() { close(_clientFd); }

void ClientConnection::processRequest() {}

bool ClientConnection::isCompleteRequest(const std::string& buffer) {
	(void)buffer;
	return true;
}

bool ClientConnection::receiveData() {
	ssize_t bytesRead = recv(_clientFd, _requestBuffer.data(), _requestBuffer.capacity(), 0);
	if (bytesRead == -1) {
		if (errno != EAGAIN) {
			LOG_ERROR("Failed to receive data: " + std::string(strerror(errno)));
			_disconnected = true;
		}
		return false;
	}

	std::string request(_requestBuffer.data(), bytesRead);
	LOG_INFO("Received request: " + request);
	return true;
}

bool ClientConnection::sendData(const std::string& data) {
	ssize_t bytesSent = send(_clientFd, data.data(), data.size(), 0);
	if (bytesSent == -1) {
		LOG_ERROR("Failed to send data: " + std::string(strerror(errno)));
		_disconnected = true;
		return false;
	}
	return true;
}

void ClientConnection::sendErrorResponse(int statusCode, const std::string& message) {
	(void)statusCode;
	(void)message;
}

bool ClientConnection::isDisconnected() const { return _disconnected; }
