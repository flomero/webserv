
#include "ClientConnection.hpp"

#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"

ClientConnection::ClientConnection(const int clientFd, const sockaddr_in clientAddr, ServerConfig& config)
	: _clientFd(clientFd), _disconnected(false), _clientAddr(clientAddr), _requestHandler(config) {
	if (fcntl(_clientFd, F_SETFL, O_NONBLOCK) == -1) {
		LOG_ERROR("Failed to set client socket to non-blocking: " + std::string(strerror(errno)));
		_disconnected = true;
		close(_clientFd);
	}
	_headerBuffer.reserve(config.getClientMaxHeaderSize());
	_bodyBuffer.reserve(config.getClientBodyBufferSize());
	LOG_INFO("Address: " + std::string(inet_ntoa(_clientAddr.sin_addr)) +
			 " Port: " + std::to_string(ntohs(_clientAddr.sin_port)));
}

ClientConnection::~ClientConnection() { close(_clientFd); }

void ClientConnection::handleClient() {
	switch (_status) {
		case Status::HEADER:
			if (!receiveHeader()) {
				return;
			}

			_status = Status::BODY;

			if (_bodyBuffer.size() ==
				static_cast<std::string::size_type>(std::stoi(_request.getHeader("Content-Length")))) {
				_status = Status::COMPLETE;
				LOG_INFO("Body is complete");
				LOG_INFO("Body: " + _bodyBuffer);
			}
			break;
		case Status::BODY:
			LOG_INFO("Hier sollte der Body verarbeitet werden");
			break;
		case Status::COMPLETE:
			processRequest();
			break;
	}
}

void ClientConnection::processRequest() {}

bool ClientConnection::receiveHeader() {
	ssize_t bytesRead = recv(_clientFd, _headerBuffer.data(), _headerBuffer.capacity(), 0);
	if (bytesRead == 0) {
		LOG_INFO("Client disconnected");
		_disconnected = true;
		return false;
	}
	if (bytesRead == -1) {
		LOG_ERROR("Failed to receive data: " + std::string(strerror(errno)));
		_disconnected = true;
		return false;
	}
	std::string request(_headerBuffer.data(), bytesRead);

	if (!isCompleteHeader(request)) {
		LOG_ERROR("Header buffer is not complete");
		_response = _requestHandler.buildDefaultResponse(Http::REQUEST_HEADER_FIELDS_TOO_LARGE);
		_status = Status::COMPLETE;
		return false;
	}

	std::string header = request.substr(0, request.find("\r\n\r\n"));
	_bodyBuffer = request.substr(request.find("\r\n\r\n") + 4);

	try {
		_request = HttpRequest(header);
	} catch (const HttpRequest::BadRequest& e) {
		_response = _requestHandler.buildDefaultResponse(Http::BAD_REQUEST);
		_status = Status::COMPLETE;
	} catch (const HttpRequest::NotImplemented& e) {
		_response = _requestHandler.buildDefaultResponse(Http::NOT_IMPLEMENTED);
		_status = Status::COMPLETE;
	} catch (const HttpRequest::InvalidVersion& e) {
		_response = _requestHandler.buildDefaultResponse(Http::HTTP_VERSION_NOT_SUPPORTED);
		_status = Status::COMPLETE;
	}

	LOG_DEBUG("Received header: " + _request.getMethod() + " " + _request.getRequestUri() + " " +
			  _request.getHttpVersion());
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

bool ClientConnection::isCompleteHeader(const std::string& buffer) {
	return buffer.find("\r\n\r\n") != std::string::npos;
}
