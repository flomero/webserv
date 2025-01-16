#include "ClientConnection.hpp"

#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>  // For std::search
#include <array>
#include <cstring>	// For strerror

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "ft_toString.hpp"

namespace {
std::string statusToString(const ClientConnection::Status status) {
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

ClientConnection::ClientConnection(const int clientFd, const sockaddr_in clientAddr, std::vector<ServerConfig> configs)
	: _clientFd(clientFd),
	  _disconnected(false),
	  _currentConfig(configs.front()),
	  _configs(std::move(configs)),
	  _clientAddr(clientAddr),
	  _requestHandler(_currentConfig) {
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

	_headerBuffer.reserve(_currentConfig.getClientHeaderBufferSize());
}

ClientConnection::~ClientConnection() {
	LOG_INFO(_log("Closing client connection"));
	if (_clientFd != -1) {
		close(_clientFd);
		_clientFd = -1;
	}
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

	LOG_DEBUG(_log("Header content: \n" + std::string(_headerBuffer.begin(), _headerBuffer.end())));
	LOG_DEBUG(_log("Header buffer size after read: " + std::to_string(_headerBuffer.size())));

	std::vector<char> header;
	if (!_extractHeaderIfComplete(header)) {
		return false;
	}

	LOG_DEBUG(_log("Header received with size: " + std::to_string(header.size())));
	LOG_DEBUG(_log("Header: \n" + std::string(header.begin(), header.end())));

	// Process the HTTP request header; handle errors by setting status and returning
	if (!_parseHttpRequestHeader(std::string(header.begin(), header.end()))) {
		_status = Status::READY_TO_SEND;
		return false;
	}
	LOG_DEBUG(_log("Parsed HTTP request header successfully"));

	if (_request.getBodyType() == HttpRequest::BodyType::NO_BODY) {
		LOG_DEBUG(_log("Request has no body"));
		_status = Status::READY_TO_SEND;
		_logHeader();
		return true;
	}

	if (_request.getBodyType() == HttpRequest::BodyType::CHUNKED ||
		_request.getBodyType() == HttpRequest::BodyType::CONTENT_LENGTH) {
		LOG_DEBUG(_log("Request has body"));
		_bodyBuffer.reserve(_currentConfig.getClientBodyBufferSize());
		_bodyBuffer.clear();
		if (_headerBuffer.empty()) {
			LOG_DEBUG(_log("No additional data in header buffer"));
		} else {
			_bodyBuffer.insert(_bodyBuffer.end(), _headerBuffer.begin(), _headerBuffer.end());
			_headerBuffer.clear();
		}

		_status = Status::BODY;
		_receiveBody();
	}

	return true;
}

ClientConnection::Status ClientConnection::getStatus() const { return _status; }

void ClientConnection::_handleCompleteChunkedBodyRead() {
	LOG_DEBUG(_log("Finished reading chunked request body"));

	LOG_DEBUG(_log("Request body: \n" + _request.getBody()));
	// _readingChunkSize = true;
	// Set the status to ready to send
	_status = Status::READY_TO_SEND;
}

bool ClientConnection::_readChunkData() {
	// Check if the entire chunk has been read
	if (_bodyBuffer.size() >= _chunkSizeRemaining) {
		// The chunk is fully read
		LOG_DEBUG(_log("Chunk fully read"));
		_request.appendToBody(std::string(_bodyBuffer.begin(), _bodyBuffer.begin() + _chunkSizeRemaining));
		_bodyBuffer.erase(_bodyBuffer.begin(), _bodyBuffer.begin() + _chunkSizeRemaining);
		return true;
	}

	LOG_DEBUG(_log("Reading chunked request data"));

	// Calculate the remaining chunk size to read
	const size_t currentChunkSize = _bodyBuffer.size();
	const size_t remainingChunkSize =
		(_chunkSizeRemaining > currentChunkSize) ? (_chunkSizeRemaining - currentChunkSize) : 0;

	// Determine the maximum bytes to read in this iteration
	const size_t maxReadSize = _requestHandler.getConfig().getClientBodyBufferSize();
	const size_t bytesToRead = std::min(remainingChunkSize, maxReadSize);

	LOG_DEBUG(_log("Attempting to read " + std::to_string(bytesToRead) + " bytes of chunk data"));

	// Read data into the body buffer; exit if read fails
	if (!_readData(_clientFd, _bodyBuffer, bytesToRead)) {
		// Read failed, return to try again later or handle error
		return false;
	}

	// If the chunk isn't fully read yet, return to wait for more data
	LOG_DEBUG(_log("Partial chunk received, waiting for remaining data"));
	return false;
}

bool ClientConnection::_readChunkTerminator() {
	LOG_DEBUG(_log("Reading chunk terminator"));
	// Attempt to find the position of the CRLF that ends the chunk data
	std::string bufferContent(_bodyBuffer.begin(), _bodyBuffer.end());
	size_t pos = bufferContent.find("\r\n");
	if (pos == std::string::npos) {
		LOG_ERROR("No CRLF found in buffer");
		// We do not have a full chunk terminator yet, attempt to read more data.

		// Try to read a minimal amount (e.g., 1 byte) to see if we can complete the chunk terminator.
		if (!_readData(_clientFd, _bodyBuffer, 1)) {
			// If no data is read, it means we don't have enough data yet.
			return false;
		}

		// If after reading data, we still don't have a complete chunk terminator, return false.
		bufferContent.assign(_bodyBuffer.begin(), _bodyBuffer.end());
		pos = bufferContent.find("\r\n");
		if (pos == std::string::npos) {
			return false;
		}
	}

	// Remove the chunk terminator from _bodyBuffer
	_bodyBuffer.erase(_bodyBuffer.begin(), _bodyBuffer.begin() + pos + 2);
	LOG_DEBUG(_log("Chunk terminator read"));
	return true;
}

void ClientConnection::_readRequestBodyIfChunked() {
	LOG_DEBUG(_log("Reading chunked request body"));

	// If we are currently reading the chunk size
	if (_readingChunkSize) {
		if (!_parseChunkSize()) {
			// If we can't parse a full chunk size yet (need more data), return to avoid blocking.
			return;
		}

		if (_chunkSizeRemaining == 0) {
			// Reached the final chunk (size = 0). The message body is complete.
			_handleCompleteChunkedBodyRead();
			return;
		}

		// Now that we have a chunk size, move on to reading chunk data.
		_readingChunkSize = false;
	}

	// If we're reading chunk data (not the chunk size)
	if (!_readingChunkSize && _chunkSizeRemaining > 0) {
		if (!_readChunkData()) {
			// If we haven't read all the chunk data yet, return and wait for more data.
			return;
		}

		// Once a chunk is fully read, it should be followed by a CRLF terminator.
		if (!_readChunkTerminator()) {
			// If the CRLF terminator isn't fully read yet, return and wait for more data.
			return;
		}

		// After successfully reading the chunk terminator, we should read the next chunk size.
		_readingChunkSize = true;
		// If `_readingChunkSize` is true, the next call to `_readRequestBodyIfChunked` will parse the next chunk size.
		if (!_bodyBuffer.empty()) {
			_readRequestBodyIfChunked();
		}
	}
}

bool ClientConnection::_parseChunkSize() {
	LOG_DEBUG(_log("Parsing chunk size"));

	// Attempt to find the position of the CRLF that ends the chunk size line.
	std::string bufferContent(_bodyBuffer.begin(), _bodyBuffer.end());
	size_t pos = bufferContent.find("\r\n");
	if (pos == std::string::npos) {
		// We do not have a full chunk size line yet, attempt to read more data.

		// Try to read a minimal amount (e.g., 1 byte) to see if we can complete the chunk size line.
		if (!_readData(_clientFd, _bodyBuffer, 1)) {
			// If no data is read, it means we don't have enough data yet.
			return false;
		}

		// If after reading data, we still don't have a complete chunk size line, return false.
		bufferContent.assign(_bodyBuffer.begin(), _bodyBuffer.end());
		pos = bufferContent.find("\r\n");
		if (pos == std::string::npos) {
			return false;
		}
	}

	// Extract the chunk size line
	std::string chunkSizeLine = bufferContent.substr(0, pos);

	// Remove the chunk size line and the CRLF from _bodyBuffer
	_bodyBuffer.erase(_bodyBuffer.begin(), _bodyBuffer.begin() + pos + 2);

	// Parse the chunk size (in hex)
	size_t chunkSize;
	std::stringstream ss;
	ss << std::hex << chunkSizeLine;
	ss >> chunkSize;

	_chunkSizeRemaining = chunkSize;
	LOG_DEBUG(_log("Parsed chunk size: " + std::to_string(chunkSize)));

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
			_readRequestBodyIfChunked();
			break;

		case HttpRequest::BodyType::NO_BODY:
			LOG_WARN(_log("No body expected for this request"));
			_status = Status::READY_TO_SEND;
			break;
	}
}

void ClientConnection::_readRequestBodyIfContentLength() {
	const size_t contentLength = _request.getContentLength();
	const size_t currentBodySize = _bodyBuffer.size();

	if (currentBodySize >= contentLength) {
		LOG_DEBUG(_log("Body buffer is already complete or has extra data"));
		_handleCompleteBodyRead();
		return;
	}

	// Calculate the remaining body size to read
	const size_t remainingBodySize = contentLength - currentBodySize;

	// Determine the maximum bytes to read in this iteration
	const size_t maxReadSize = _requestHandler.getConfig().getClientBodyBufferSize();
	const size_t bytesToRead = std::min(remainingBodySize, maxReadSize);

	LOG_DEBUG(_log("Attempting to read " + std::to_string(bytesToRead) + " bytes of body data"));

	// Read data into the body buffer; exit if read fails
	if (!_readData(_clientFd, _bodyBuffer, bytesToRead)) {
		// Read failed, return to try again later or handle error
		return;
	}

	LOG_DEBUG(_log("Body buffer size after read: " + std::to_string(_bodyBuffer.size())));

	// Check if the entire body has been read
	if (_bodyBuffer.size() >= contentLength) {
		_handleCompleteBodyRead();
	} else {
		LOG_DEBUG(_log("Partial body received, waiting for remaining data"));
	}
}

void ClientConnection::_handleCompleteBodyRead() {
	// Handle any extra data read beyond content length
	const size_t contentLength = _request.getContentLength();
	if (_bodyBuffer.size() > contentLength) {
		// Move extra data into the header buffer for potential next request
		_headerBuffer.assign(_bodyBuffer.begin() + contentLength, _bodyBuffer.end());
		LOG_ERROR(_log("Extra data read beyond content length"));
		// Resize body buffer to the exact content length
		_bodyBuffer.resize(contentLength);
	}

	// Set the complete body in the request
	_request.setBody(std::string(_bodyBuffer.begin(), _bodyBuffer.end()));

	// Log the completed header and body info
	_logHeader();

	// Update status to ready
	_status = Status::READY_TO_SEND;
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

	header = std::vector(_headerBuffer.begin(), _headerBuffer.begin() + header_end_pos + 1);
	header.push_back('\0');	 // Null-terminate the header

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

	bool isKnownHost = false;
	// Check if there is a matching server config
	for (const auto& config : _configs) {
		for (const auto& serverName : config.getServerNames()) {
			if (serverName + ":" + std::to_string(config.getPort()) == _request.getHeader("Host") ||
				serverName == _request.getHeader("Host")) {
				_currentConfig = config;
				_requestHandler.setConfig(_currentConfig);
				isKnownHost = true;
				LOG_INFO(_log("Server config found for host: " + _request.getHeader("Host")));
				break;
			}
		}
		if (isKnownHost) {
			break;
		}
	}

	if (!isKnownHost) {
		LOG_WARN(_log("No server config found for host: " + _request.getHeader("Host")));
		_response = HttpResponse(Http::BAD_REQUEST);
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
		_response = _requestHandler.buildDefaultResponse(Http::PAYLOAD_TOO_LARGE);
		_status = Status::READY_TO_SEND;
		// _disconnected = true;
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

std::string ClientConnection::_log(const std::string& msg) const {
	static const std::array<std::string, 6> colors = {CYAN, GREEN, YELLOW, BLUE, MAGENTA, ORANGE};
	const std::string& color = colors[_clientFd % colors.size()];

	return COLOR(color, "ClientCon " + std::to_string(_clientFd)) + "\t | " + msg;
}
