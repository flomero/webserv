
#include "ClientConnection.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

ClientConnection::ClientConnection(int fd)
	: _clientFd(fd), _disconnected(false) {}

ClientConnection::~ClientConnection() { close(_clientFd); }

void ClientConnection::processRequest() {
	while (!_disconnected) {
		if (!receiveData())
			break;

		try {
			while (isCompleteRequest(_requestBuffer)) {
				HttpRequest request(_requestBuffer);

				HttpResponse response;
				response.setStatus(200); // OK
				response.setDefaultHeaders();

				response.setBody("<html><body><h1>Success</h1></body></html>");

				if(!sendData(response.toString())) {
					break;;
				}

				_requestBuffer.erase(0, request.getRequestLength());
				if(request.getHeader("Connection") == "close") {
					_disconnected = true;
					break;
				}
			}
		} catch (const HttpRequest::BadRequest& e) {
			sendErrorResponse(400, "Bad Request");
			_disconnected = true;
			break;
		} catch (const HttpRequest::NotImplemented& e) {
			sendErrorResponse(501, "Not Implemented");
			_disconnected = true;
			break;
		} catch (const HttpRequest::InvalidVersion& e) {
			sendErrorResponse(505, "HTTP Version Not Supported");
			_disconnected = true;
			break;
		}

	}
}

bool ClientConnection::isCompleteRequest(const std::string& buffer) {
	// Simple check for the end of headers ("\r\n\r\n")
	return buffer.find("\r\n\r\n") != std::string::npos;
}

bool ClientConnection::receiveData() {
	char buffer[4096];
	ssize_t bytesReceived = read(_clientFd, buffer, sizeof(buffer));

	if (bytesReceived == 0) {
		// Client closed the connection
		_disconnected = true;
		return false;
	}
	if (bytesReceived < 0) {
		// Error occurred
		perror("Read failed");
		_disconnected = true;
		return false;
	}
	// Append data to the request buffer
	_requestBuffer.append(buffer, bytesReceived);
	return true;
}

bool ClientConnection::sendData(const std::string& data) {
	size_t totalSent = 0;
	size_t dataSize = data.size();

	while (totalSent < dataSize) {
		ssize_t bytesSent = send(_clientFd, data.c_str() + totalSent, dataSize - totalSent, 0);
		if (bytesSent <= 0) {
			perror("Send failed");
			_disconnected = true;
			return false;
		}
		totalSent += bytesSent;
	}
	return true;
}

void ClientConnection::sendErrorResponse(int statusCode, const std::string& message) {
	HttpResponse response(statusCode);
	response.setDefaultHeaders();
	response.setBody(message);
	sendData(response.toString());
}

bool ClientConnection::isDisconnected() const {
	return _disconnected;
}
