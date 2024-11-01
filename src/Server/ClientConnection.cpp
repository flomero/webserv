
#include "ClientConnection.hpp"

#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "ft_toString.hpp"

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
			receiveBody();
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

	std::vector<char> header;
	if (!_extractHeaderIfComplete(header))
		return false;

	// Process the HTTP request header; handle errors by setting status and returning
	if (!_processHttpRequest(header.data())) {
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
			if (_headerBuffer.empty()) {
				break;
			}
			if (_headerBuffer.size() >= contentLength) {
				// If buffer has full body content, set body and update status
				_bodyBuffer.insert(_bodyBuffer.end(), _headerBuffer.begin(), _headerBuffer.begin() + contentLength);
				_headerBuffer.erase(_headerBuffer.begin(), _headerBuffer.begin() + contentLength);
				_request.setBody(_bodyBuffer.data());
				_status = Status::READY_TO_SEND;
				_logHeader();
			} else {
				// If partial body content, reserve space and wait for more
				_bodyBuffer.reserve(contentLength);
				_bodyBuffer = _headerBuffer;
				_headerBuffer.clear();
				_status = Status::BODY;
				// LOG_DEBUG("Body buffer: " + _bodyBuffer);
			}
			break;
		}

		case HttpRequest::BodyType::CHUNKED:
			LOG_ERROR("Chunked body not implemented");
			break;
	}

	return true;
}

void ClientConnection::receiveBody() {
	switch (_request.getBodyType()) {
		case HttpRequest::BodyType::CONTENT_LENGTH: {
			// Determine the remaining body size and the max bytes to read this iteration
			const size_t contentLength = _request.getContentLength();
			size_t remainingBodySize = contentLength - _bodyBuffer.size();
			size_t bytesToRead = std::min(remainingBodySize, _requestHandler.getConfig().getClientBodyBufferSize());

			// Read data into the body buffer; exit if read fails
			if (!_readData(_clientFd, _bodyBuffer, bytesToRead)) {
				return;
			}

			// Check if the entire body has been read
			if (_bodyBuffer.size() == contentLength) {
				_request.setBody(_bodyBuffer.data());  // Set the complete body in the request
				_logHeader();						   // Log the completed header and body info
				_status = Status::READY_TO_SEND;	   // Update status to ready
			}
			break;
		}

		case HttpRequest::BodyType::CHUNKED:
			// Log an error as chunked transfer encoding is not yet implemented
			LOG_ERROR("Chunked body not implemented");
			break;

		case HttpRequest::BodyType::NO_BODY:
			// Log a warning as no body content is expected for this request
			LOG_WARN("No body expected");
			break;
	}
}

bool ClientConnection::_extractHeaderIfComplete(std::vector<char>& header) {
	// Check if header is complete and get the position of the end
	const auto header_end_index = _findHeaderEnd(_headerBuffer);

	if (!header_end_index) {
		// Header not complete
		if (_headerBuffer.size() > _requestHandler.getConfig().getClientMaxHeaderSize()) {
			LOG_ERROR("Header size exceeds maximum allowed size");
			_response = _requestHandler.buildDefaultResponse(Http::REQUEST_HEADER_FIELDS_TOO_LARGE);
			_disconnected = true;
			return false;
		}
		// Header not complete but size acceptable; wait for more data
		return false;
	}

	// Convert header_end_index to the signed type required for vector operations
	const auto header_end_pos = static_cast<std::vector<char>::difference_type>(*header_end_index);

	header = std::vector(_headerBuffer.begin(), _headerBuffer.begin() + header_end_pos);

	// Erase the header and the delimiter from _headerBuffer
	_headerBuffer.erase(_headerBuffer.begin(), _headerBuffer.begin() + header_end_pos + 4);
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
	if (_request.getHeader("Connection") == "keep-alive") {
		_status = Status::HEADER;
		_disconnected = false;
	} else {
		_disconnected = true;
	}
}

bool ClientConnection::_readData(const int fd, std::vector<char>& buffer, const size_t bytesToRead) {
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
	buffer.insert(buffer.end(), tmp.begin(), tmp.begin() + bytesRead);
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

std::optional<size_t> ClientConnection::_findHeaderEnd(const std::vector<char>& buffer) {
	const std::string pattern = "\r\n\r\n";
	if (const auto position = std::search(buffer.begin(), buffer.end(), pattern.begin(), pattern.end());
		position != buffer.end()) {
		return std::distance(buffer.begin(), position);	 // Return the index position
	}
	return std::nullopt;  // Return std::nullopt if not found
}

bool ClientConnection::isDisconnected() const { return _disconnected; }

void ClientConnection::_logHeader() const {
	LOG_DEBUG("\n");
	LOG_DEBUG("=== Request Received ===");
	LOG_DEBUG("\n" + toString(_request));
	LOG_DEBUG("=======================\n");
}
