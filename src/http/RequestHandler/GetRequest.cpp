/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/31 14:35:29 by flfische          #+#    #+#             */
/*   Updated: 2024/11/01 17:53:39 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

#include <fstream>

#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include "mimetypes.hpp"

HttpResponse RequestHandler::handleGetRequest() {
	LOG_DEBUG("Handling GET request");
	HttpResponse response;

	if (std::filesystem::is_directory(_request.getServerSidePath())) {
		response = handleGetDirectory();
	} else {
		response = handleGetFile();
	}

	return response;
}

HttpResponse RequestHandler::handleGetFile() {
	HttpResponse response;

	int fd = open(_request.getServerSidePath().c_str(), O_RDONLY);
	if (fd == -1) {
		return buildDefaultResponse(Http::Status::NOT_FOUND);
	}

	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;

	int timeout = 5000;	 // TODO: make this configurable
	int ret = poll(&pfd, 1, timeout);

	if (ret <= 0 || !(pfd.revents & POLLIN)) {
		close(fd);
		return buildDefaultResponse(Http::REQUEST_TIMEOUT);
	}

	std::ifstream file(_request.getServerSidePath(), std::ios::binary);
	if (!file.is_open()) {
		close(fd);
		return buildDefaultResponse(Http::NOT_FOUND);
	}

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string content(fileSize, '\0');
	file.read(&content[0], content.size());
	file.close();
	close(fd);

	// Set up the HTTP response
	response.setBody(content);
	response.addHeader("Content-Type", getMimeType(_request.getServerSidePath()));
	response.addHeader("Content-Length", std::to_string(fileSize));
	response.setStatus(Http::OK);

	return response;
}

HttpResponse RequestHandler::handleGetDirectory() {
	HttpResponse response;

	// check index file
	std::string indexPath = _request.getServerSidePath() + "/" + _serverConfig.getIndex();
	if (std::filesystem::exists(indexPath)) {
		_request.setServerSidePath(indexPath);
		return handleGetFile();
	}

	// autoindex
	LOG_DEBUG("Autoindex is " + std::string(_matchedRoute.isAutoindex() ? "enabled" : "disabled"));
	LOG_DEBUG(_matchedRoute.getPath());
	if (_matchedRoute.isAutoindex()) {
		handleAutoindex(_request.getServerSidePath());
		return _response;
	}
	LOG_INFO("Directory listing is disabled");
	return buildDefaultResponse(Http::FORBIDDEN);
}