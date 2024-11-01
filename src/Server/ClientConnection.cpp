
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
			break;
		case Status::BODY:
			LOG_INFO("Hier sollte der Body verarbeitet werden");
			break;
		case Status::READY_TO_SEND:
			break;
	}
}

bool ClientConnection::receiveHeader() {
	// Attempt to read data into the header buffer
	size_t remainingHeaderSize = _requestHandler.getConfig().getClientMaxHeaderSize() - _headerBuffer.size();
	if (!_readData(_clientFd, _headerBuffer, remainingHeaderSize)) {
		return false;
	}

	// Check if header is complete, handle error if not
	if (!_isHeaderComplete(_headerBuffer)) {
		LOG_ERROR("Header buffer is not complete");
		_response = _requestHandler.buildDefaultResponse(Http::REQUEST_HEADER_FIELDS_TOO_LARGE);
		_disconnected = true;
		return false;
	}

	// Extract header content and remove from buffer
	std::string header = _headerBuffer.substr(0, _headerBuffer.find("\r\n\r\n"));
	_headerBuffer.erase(0, header.size() + 4);

	// Process the HTTP request header; handle errors by setting status and returning
	if (!_processHttpRequest(header)) {
		_status = Status::READY_TO_SEND;
		return false;
	}

	// Determine body handling based on header body type
	switch (_request.getBodyType()) {
		case HttpRequest::BodyType::NO_BODY:
			_status = Status::READY_TO_SEND;
			break;

		case HttpRequest::BodyType::CONTENT_LENGTH: {
			// Handle cases with data already in buffer
			const size_t contentLength = _request.getContentLength();
			if (!_headerBuffer.empty()) {
				if (_headerBuffer.size() >= contentLength) {
					// If buffer has full body content, set body and update status
					_bodyBuffer = _headerBuffer.substr(0, contentLength);
					_headerBuffer.erase(0, contentLength);
					_request.setBody(_bodyBuffer);
					_status = Status::READY_TO_SEND;
				} else {
					// If partial body content, reserve space and wait for more
					_bodyBuffer.reserve(contentLength);
					_bodyBuffer = _headerBuffer;
					_headerBuffer.clear();
					_status = Status::BODY;
				}
			}
			break;
		}

		case HttpRequest::BodyType::CHUNKED:
			LOG_ERROR("Chunked body not implemented");
			break;
	}

	_logHeader();
	return true;
}

bool ClientConnection::_processHttpRequest(const std::string& header) {
	try {
		_request = HttpRequest(header);
	} catch (const HttpRequest::BadRequest& e) {
		_response = _requestHandler.buildDefaultResponse(Http::BAD_REQUEST);
		return false;
	} catch (const HttpRequest::NotImplemented& e) {
		_response = _requestHandler.buildDefaultResponse(Http::NOT_IMPLEMENTED);
		return false;
	} catch (const HttpRequest::InvalidVersion& e) {
		_response = _requestHandler.buildDefaultResponse(Http::HTTP_VERSION_NOT_SUPPORTED);
		return false;
	}
	return true;
}

void ClientConnection::sendResponse() {
	if (_status != Status::READY_TO_SEND) {
		// LOG_ERROR("Response is not complete");
		return;
	}
	_response = _requestHandler.handleRequest(_request);
	LOG_INFO("Sending response");
	LOG_DEBUG("Response: \n" + _response.toString());
	if (!_sendDataToClient(_response.toString())) {
		return;
	}
	_status = Status::HEADER;
	_disconnected = true;  // TODO check when to keep open
}

bool ClientConnection::_readData(const int fd, std::string& buffer, const size_t bytesToRead) {
	if (buffer.capacity() < bytesToRead) {
		LOG_ERROR("Buffer is too small");
		_disconnected = true;
		return false;
	}
	std::vector<char> tmp(bytesToRead);

	const ssize_t bytesRead = recv(fd, tmp.data(), bytesToRead, 0);
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
	buffer.append(tmp.data(), bytesRead);
	return true;
}

bool ClientConnection::_sendDataToClient(const std::string& data) {
	// TODO make chunked
	ssize_t bytesSent = send(_clientFd, data.data(), data.size(), 0);
	if (bytesSent == -1) {
		LOG_ERROR("Failed to send data: " + std::string(strerror(errno)));
		_disconnected = true;
		return false;
	}

	return true;
}

bool ClientConnection::_isHeaderComplete(const std::string& buffer) {
	return buffer.find("\r\n\r\n") != std::string::npos;
}

bool ClientConnection::isDisconnected() const { return _disconnected; }

void ClientConnection::_logHeader() const {
	LOG_DEBUG("\n");
	LOG_DEBUG("=== Request Received ===");
	LOG_DEBUG("Header:");
	LOG_DEBUG(" |- Method: " + _request.getMethod());
	LOG_DEBUG(" |- URI: " + _request.getRequestUri());
	LOG_DEBUG(" |- Version: " + _request.getHttpVersion());
	if (_request.getBodyType() != HttpRequest::BodyType::NO_BODY) {
		LOG_DEBUG("Body: " + _request.getBody());
	}

	LOG_DEBUG("=======================\n");
}
