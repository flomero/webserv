/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/11 14:43:11 by flfische          #+#    #+#             */
/*   Updated: 2025/01/17 18:16:06 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <fstream>

#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include "webserv.hpp"

bool RequestHandler::handlePostRequest() {
	LOG_DEBUG("Handling POST request");
	const std::string contentType = _request.getHeader("Content-Type");
	if (contentType == "application/x-www-form-urlencoded") {
		LOG_INFO("application/x-www-form-urlencoded");
		// TODO: not sure if needed
	} else if (contentType.find("multipart/form-data") != std::string::npos)
		return handlePostMultipart();

	LOG_ERROR("Unsupported content type: " + contentType);
	return buildDefaultResponse(Http::UNSUPPORTED_MEDIA_TYPE);
}

HttpResponse RequestHandler::handlePostMultipart() {
	LOG_DEBUG("Handling multipart POST request");
	std::string contentType = _request.getHeader("Content-Type");
	std::size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos) {
		LOG_ERROR("Invalid Content-Type header: " + contentType);
		return buildDefaultResponse(Http::BAD_REQUEST);
	}
	std::string boundary = contentType.substr(boundaryPos + 9);
	if (boundary.empty()) {
		LOG_ERROR("Invalid boundary in Content-Type header: " + contentType);
		return buildDefaultResponse(Http::BAD_REQUEST);
	}
	std::string boundaryDelimiter = "--" + boundary;
	std::string boundaryEnd = boundaryDelimiter + "--";
	std::size_t pos = 0;

	std::string body = _request.getBody();
	while ((pos = body.find(boundaryDelimiter, pos)) != std::string::npos) {
		pos += boundaryDelimiter.length();
		if (pos == body.size())
			break;
		std::size_t endPos = body.find(boundaryDelimiter, pos);
		std::string part = body.substr(pos, endPos - pos);
		pos = endPos;

		std::istringstream partStream(part);
		std::string line;
		std::string contentDisposition;
		std::string contentTypeFile;
		while (std::getline(partStream, line) && !line.empty()) {
			if (line.find("Content-Disposition") == 0)
				contentDisposition = line;
			else if (line.find("Content-Type") == 0)
				contentTypeFile = line;
		}
		if (contentDisposition.empty()) {
			LOG_ERROR("Missing Content-Disposition header in part");
			return buildDefaultResponse(Http::BAD_REQUEST);
		}
		if (contentTypeFile.empty()) {
			LOG_ERROR("Missing Content-Type header in part");
			return buildDefaultResponse(Http::BAD_REQUEST);
		}
		if (contentDisposition.find("filename=") != std::string::npos) {
			LOG_DEBUG("Handling file upload");
			return handleFileUpload(part, contentDisposition);
		}

		// TODO: handle form fields - not sure if needed
	}
	return buildDefaultResponse(Http::OK);
}

std::string buildpath(const std::string &path, const std::string &filename, const std::string &root) {
	std::string result = root;
	if (result.front() != '.')
		result = "." + result;
	if (result.back() != '/')
		result += '/';
	if (path.front() == '/')
		result += path.substr(1);
	else
		result += path;
	if (result.back() != '/')
		result += '/';
	if (filename.front() == '/')
		result += filename.substr(1);
	else
		result += filename;
	return result;
}

HttpResponse RequestHandler::handleFileUpload(const std::string &part, const std::string &contentDisposition) {
	std::string filename;
	std::size_t filenamePos = contentDisposition.find("filename=");
	if (filenamePos != std::string::npos) {
		filename = contentDisposition.substr(filenamePos + 10);
		filename = filename.substr(0, filename.find('\"'));
	}
	if (filename.empty()) {
		LOG_ERROR("Invalid filename in Content-Disposition header: " + contentDisposition);
		return buildDefaultResponse(Http::BAD_REQUEST);
	}
	std::string fileContent;
	std::size_t pos = part.find("\r\n\r\n");
	if (pos != std::string::npos)
		fileContent = part.substr(pos + 4);
	else {
		LOG_ERROR("Invalid part: " + part);
		return buildDefaultResponse(Http::BAD_REQUEST);
	}
	if (!_matchedRoute.getUploadDir().empty()) {
		if (!_matchedRoute.getRoot().empty())
			filename = buildpath(_matchedRoute.getUploadDir(), filename, _matchedRoute.getRoot());
		else
			filename = buildpath(_matchedRoute.getUploadDir(), filename, _serverConfig.getRoot());
	} else {
		if (!_serverConfig.getUploadDir().empty())
			filename = buildpath(_serverConfig.getUploadDir(), filename, _serverConfig.getRoot());
		else
			return buildDefaultResponse(Http::FORBIDDEN);
	}
	int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		LOG_ERROR("Failed to open file descriptor for: " + filename + " with error: " + strerror(errno));
		return buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
	}

	struct pollfd pfd{};
	pfd.fd = fd;
	pfd.events = POLLOUT;

	int ret = poll(&pfd, 1, DEFAULT_POLL_TIMEOUT);
	if (ret < 0) {
		LOG_ERROR("Poll error for file: " + filename + " with error: " + strerror(errno));
		close(fd);
		return buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
	} else if (ret == 0) {
		LOG_WARN("Poll timeout for file: " + filename);
		close(fd);
		return buildDefaultResponse(Http::REQUEST_TIMEOUT);
	} else if (!(pfd.revents & POLLOUT)) {
		LOG_WARN("Unexpected poll revents for file: " + filename + " revents: " + std::to_string(pfd.revents));
		close(fd);
		return buildDefaultResponse(Http::REQUEST_TIMEOUT);
	}

	ssize_t written = write(fd, fileContent.c_str(), fileContent.size());
	if (written < 0) {
		LOG_ERROR("Failed to write to file: " + filename + " with error: " + strerror(errno));
		close(fd);
		return buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
	}

	close(fd);
	LOG_INFO("File uploaded successfully: " + filename);
	return buildDefaultResponse(Http::CREATED);
}
