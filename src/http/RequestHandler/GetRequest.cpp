/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/31 14:35:29 by flfische          #+#    #+#             */
/*   Updated: 2025/01/17 23:13:39 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

#include <fstream>

#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include "mimetypes.hpp"
#include "webserv.hpp"

bool RequestHandler::handleGetRequest() {
	LOG_DEBUG("Handling GET request");

	if (std::filesystem::is_directory(_request.getServerSidePath())) {
		return handleGetDirectory();
	}
	return handleGetFile();
}

bool RequestHandler::handleGetFile() {
	LOG_ERROR("Try to open file: " + _request.getServerSidePath());
	const int fd = open(_request.getServerSidePath().c_str(), O_RDONLY);
	if (fd == -1) {
		_response = buildDefaultResponse(Http::Status::IM_A_TEAPOT);
		return true;
	}

	pollfd pfd{};
	pfd.fd = fd;
	pfd.events = POLLIN;

	if (const int ret = poll(&pfd, 1, DEFAULT_POLL_TIMEOUT); ret <= 0 || !(pfd.revents & POLLIN)) {
		close(fd);
		_response = buildDefaultResponse(Http::REQUEST_TIMEOUT);
		return true;
	}

	std::ifstream file(_request.getServerSidePath(), std::ios::binary);
	if (!file.is_open()) {
		close(fd);
		_response = buildDefaultResponse(Http::PAYLOAD_TOO_LARGE);
		return true;
	}

	file.seekg(0, std::ios::end);
	const long long fileSize = file.tellg();
	file.seekg(_bytesReadFromFile, std::ios::beg);	// Set the offset

	std::string content;
	content.reserve(fileSize);

	char buffer[GET_READ_SIZE];

	file.read(buffer, GET_READ_SIZE);
	const size_t readSize = file.gcount();
	content.append(buffer, readSize);
	// LOG_ERROR("Content: " + content);
	_bytesReadFromFile += readSize;
	_response.appendToBody(content);
	LOG_DEBUG("Read " + std::to_string(readSize) +
			  " bytes from file. Remaining: " + std::to_string(fileSize - _bytesReadFromFile));

	file.close();
	close(fd);

	if (_bytesReadFromFile >= fileSize) {
		// Set up the HTTP response
		_response.addHeader("Content-Type", getMimeType(_request.getServerSidePath()));
		_response.addHeader("Content-Length", std::to_string(fileSize));
		_response.setStatus(Http::OK);
	}

	return _bytesReadFromFile >= fileSize;
}

bool RequestHandler::handleGetDirectory() {
	// check index file
	const std::string indexPath = _request.getServerSidePath() + "/" + _serverConfig.getIndex();
	if (std::filesystem::exists(indexPath)) {
		_request.setServerSidePath(indexPath);
		return handleGetFile();
	}

	// autoindex
	LOG_DEBUG("Autoindex is " + std::string(_matchedRoute.isAutoindex() ? "enabled" : "disabled"));
	LOG_DEBUG(_matchedRoute.getPath());
	if (_matchedRoute.isAutoindex()) {
		handleAutoindex(_request.getServerSidePath());
		return true;
	}
	LOG_INFO("Directory listing is disabled");
	_response = buildDefaultResponse(Http::FORBIDDEN);
	return true;
}
