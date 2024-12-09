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

namespace {
std::string statusToString(ClientConnection::Status status) {
	switch (status) {
		case ClientConnection::Status::HEADER:
			return "HEADER";
		case ClientConnection::Status::BODY:
			return "BODY";
		case ClientConnection::Status::READY_TO_SEND:
			return "READY_TO_SEND";
		default:
			return "UNKNOWN";
	}
}
}  // namespace

ClientConnection::ClientConnection(const int clientFd, const sockaddr_in clientAddr, ServerConfig& config)
	: _clientFd(clientFd), _disconnected(false), _clientAddr(clientAddr), _requestHandler(config) {
	LOG_INFO(_log("New client connection established"));
	LOG_INFO("Client address: " + std::string(inet_ntoa(_clientAddr.sin_addr)) +
			 " Port: " + std::to_string(ntohs(_clientAddr.sin_port)));

	if (fcntl(_clientFd, F_SETFL, O_NONBLOCK) == -1) {
		LOG_ERROR(_log("Failed to set client socket to non-blocking: " + std::string(strerror(errno))));
		_disconnected = true;
		close(_clientFd);
	} else {
		LOG_DEBUG(_log("Client socket set to non-blocking mode"));
	}

	_headerBuffer.reserve(config.getClientHeaderBufferSize());
	_bodyBuffer.reserve(config.getClientBodyBufferSize());
}

ClientConnection::~ClientConnection() {
	LOG_INFO(_log("Closing client connection"));
	close(_clientFd);
}

void ClientConnection::handleClient() {
	LOG_DEBUG(_log("Handling client with status: " + statusToString(_status)));
	switch (_status) {
		case Status::HEADER:
			_receiveHeader();
			break;
		case Status::BODY:
			_receiveBody();
			break;
		case Status::READY_TO_SEND:
			// Nothing to do here
			break;
	}
}

bool ClientConnection::_receiveHeader() {
	LOG_DEBUG(_log("Receiving header from client"));
	// Attempt to read data into the header buffer
	size_t remainingHeaderSize = _requestHandler.getConfig().getClientHeaderBufferSize() - _headerBuffer.size();
	if (!_readData(_clientFd, _headerBuffer, remainingHeaderSize)) {
		return false;
	}
	LOG_DEBUG(_log("Header buffer size after read: " + std::to_string(_headerBuffer.size())));

	std::vector<char> header;
	if (!_extractHeaderIfComplete(header)) {
		return false;
	}

	LOG_DEBUG(_log("Header received with size: " + std::to_string(header.size())));
	LOG_DEBUG(_log("Header: \n" + std::string(header.begin(), header.end())));

	// Process the HTTP request header; handle errors by setting status and returning
	if (!_parseHttpRequestHeader(header.data())) {
		_status = Status::READY_TO_SEND;
		return false;
	}
	LOG_DEBUG(_log("Parsed HTTP request header successfully"));

	// Determine body handling based on header body type
	switch (_request.getBodyType()) {
		case HttpRequest::BodyType::NO_BODY:
			LOG_DEBUG(_log("Request has no body"));
			_status = Status::READY_TO_SEND;
			_logHeader();
			break;

		case HttpRequest::BodyType::CONTENT_LENGTH: {
			LOG_DEBUG(_log("Request has body with Content-Length: " + std::to_string(_request.getContentLength())));
			// Handle cases with data already in buffer
			const size_t contentLength = _request.getContentLength();
			if (_headerBuffer.empty()) {
				LOG_DEBUG(_log("No additional data in header buffer"));
				_bodyBuffer.reserve(contentLength);
				_status = Status::BODY;
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
				LOG_DEBUG(_log("Partial body received, waiting for remaining data"));
			}
			break;
		}

		case HttpRequest::BodyType::CHUNKED:
			LOG_ERROR(_log("Chunked body not implemented"));
			_response = _requestHandler.buildDefaultResponse(Http::NOT_IMPLEMENTED);
			_status = Status::READY_TO_SEND;
			break;
	}

	return true;
}

void ClientConnection::_receiveBody() {
	LOG_DEBUG(_log("Receiving body from client"));
	switch (_request.getBodyType()) {
		case HttpRequest::BodyType::CONTENT_LENGTH: {
			_readRequestBodyIfContentLength();
			break;
		}

		case HttpRequest::BodyType::CHUNKED:
			LOG_ERROR(_log("Chunked body not implemented"));
			_response = _requestHandler.buildDefaultResponse(Http::NOT_IMPLEMENTED);
			_status = Status::READY_TO_SEND;
			break;

		case HttpRequest::BodyType::NO_BODY:
			LOG_WARN(_log("No body expected for this request"));
			_status = Status::READY_TO_SEND;
			break;
	}
}

void ClientConnection::_readRequestBodyIfContentLength() {
	// Determine the remaining body size and the max bytes to read this iteration
	const size_t contentLength = _request.getContentLength();
	const size_t remainingBodySize = contentLength - _bodyBuffer.size();
	const size_t bytesToRead = std::min(remainingBodySize, _requestHandler.getConfig().getClientBodyBufferSize());

	LOG_DEBUG(_log("Attempting to read " + std::to_string(bytesToRead) + " bytes of body data"));

	// Read data into the body buffer; exit if read fails
	if (!_readData(_clientFd, _bodyBuffer, bytesToRead)) {
		return;
	}

	LOG_DEBUG(_log("Body buffer size after read: " + std::to_string(_bodyBuffer.size())));

	// Check if the entire body has been read
	if (_bodyBuffer.size() == contentLength) {
		_request.setBody(_bodyBuffer.data());  // Set the complete body in the request
		_logHeader();						   // Log the completed header and body info
		_status = Status::READY_TO_SEND;	   // Update status to ready
	} else {
		LOG_DEBUG(_log("Partial body received, waiting for remaining data"));
	}
}

bool ClientConnection::_extractHeaderIfComplete(std::vector<char>& header) {
	// Check if header is complete and get the position of the end
	const auto headerEndIndex = _findHeaderEnd(_headerBuffer);

	if (!headerEndIndex) {
		// Header not complete
		if (_headerBuffer.size() > _requestHandler.getConfig().getClientHeaderBufferSize()) {
			LOG_ERROR(_log("Header size exceeds maximum allowed size"));
			_response = _requestHandler.buildDefaultResponse(Http::REQUEST_HEADER_FIELDS_TOO_LARGE);
			_disconnected = true;
			return false;
		}
		// Header not complete but size acceptable; wait for more data
		LOG_DEBUG(_log("Header not complete, waiting for more data"));
		return false;
	}

	// Convert header_end_index to the signed type required for vector operations
	const auto header_end_pos = static_cast<std::vector<char>::difference_type>(*headerEndIndex);

	header = std::vector(_headerBuffer.begin(), _headerBuffer.begin() + header_end_pos);

	// Erase the header and the delimiter from _headerBuffer
	_headerBuffer.erase(_headerBuffer.begin(), _headerBuffer.begin() + header_end_pos + 4);
	LOG_DEBUG(_log("Header extracted from buffer"));
	return true;
}

bool ClientConnection::_parseHttpRequestHeader(const std::string& header) {
	try {
		_request = HttpRequest(header);
	} catch (const HttpRequest::BadRequest& e) {
		LOG_WARN(std::string(e.what()));
		_response = _requestHandler.buildDefaultResponse(Http::BAD_REQUEST);
		return false;
	} catch (const HttpRequest::NotImplemented& e) {
		LOG_WARN(std::string(e.what()));
		_response = _requestHandler.buildDefaultResponse(Http::NOT_IMPLEMENTED);
		return false;
	} catch (const HttpRequest::InvalidVersion& e) {
		LOG_WARN(std::string(e.what()));
		_response = _requestHandler.buildDefaultResponse(Http::HTTP_VERSION_NOT_SUPPORTED);
		return false;
	}
	return true;
}

void ClientConnection::sendResponse() {
	if (_status != Status::READY_TO_SEND) {
		// LOG_ERROR("Response is not ready to be sent");
		return;
	}
	if (!_response.getStatus()) {
		LOG_DEBUG(_log("Building response for request"));
		_response = _requestHandler.handleRequest(_request);
	}
	LOG_INFO(_log("Sending response with status code: " + std::to_string(_response.getStatus())));
	LOG_DEBUG(_log("Response: \n" + _response.toString()));
	if (!_sendDataToClient(_response.toString())) {
		return;
	}
	if (_response.getHeader("Connection") == "keep-alive") {
		LOG_INFO(_log("Connection is keep-alive"));
		_status = Status::HEADER;
		_disconnected = false;
		_response = HttpResponse();
	} else {
		LOG_INFO(_log("Closing connection after response"));
		_disconnected = true;
	}
}

bool ClientConnection::_readData(const int fd, std::vector<char>& buffer, const size_t bytesToRead) {
	if (buffer.capacity() < buffer.size() + bytesToRead) {
		LOG_ERROR(_log("Buffer capacity is insufficient"));
		_disconnected = true;
		return false;
	}
	std::vector<char> tmp(bytesToRead);

	const ssize_t bytesRead = recv(fd, tmp.data(), bytesToRead, 0);
	if (bytesRead == 0) {
		LOG_INFO(_log("Client disconnected"));
		_disconnected = true;
		return false;
	}
	if (bytesRead == -1) {
		LOG_ERROR(_log("Failed to receive data: " + std::string(strerror(errno))));
		_disconnected = true;
		return false;
	}
	buffer.insert(buffer.end(), tmp.begin(), tmp.begin() + bytesRead);
	LOG_DEBUG(_log("Read " + std::to_string(bytesRead) + " bytes"));
	return true;
}

bool ClientConnection::_sendDataToClient(const std::string& data) {
	// TODO: Implement chunked transfer encoding if necessary
	ssize_t bytesSent = send(_clientFd, data.data(), data.size(), 0);
	if (bytesSent == -1) {
		LOG_ERROR(_log("Failed to send data: " + std::string(strerror(errno))));
		_disconnected = true;
		return false;
	}
	LOG_DEBUG(_log("Sent " + std::to_string(bytesSent) + " bytes to client"));
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
	LOG_DEBUG(_log("Request recieved:\n====================\n" + toString(_request) + "\n===================="));
}

std::string ClientConnection::_log(std::string msg) const {
	std::string color;
	int x = _clientFd % 6;

	switch (x) {
		case 0:
			color = CYAN;
			break;
		case 1:
			color = GREEN;
			break;
		case 2:
			color = YELLOW;
			break;
		case 3:
			color = BLUE;
			break;
		case 4:
			color = MAGENTA;
			break;
		case 5:
			color = ORANGE;
			break;
		default:
			color = "";
			break;
	}

	return COLOR(color, "ClientCon " + std::to_string(_clientFd)) + "\t | " + msg;
}