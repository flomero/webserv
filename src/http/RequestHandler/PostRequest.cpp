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
	} else if (contentType.find("multipart/form-data") != std::string::npos)
		return handlePostMultipart();

	LOG_ERROR("Unsupported content type: " + contentType);
	_response = buildDefaultResponse(Http::UNSUPPORTED_MEDIA_TYPE);
	return true;
}

bool RequestHandler::handlePostMultipart() {
	LOG_DEBUG("Handling multipart POST request");
	if (_fileName.empty()) {
		LOG_DEBUG("No file name set yet");
		std::string contentType = _request.getHeader("Content-Type");
		std::size_t boundaryPos = contentType.find("boundary=");
		if (boundaryPos == std::string::npos) {
			LOG_ERROR("Invalid Content-Type header: " + contentType);
			_response = buildDefaultResponse(Http::BAD_REQUEST);
			return true;
		}
		std::string boundary = contentType.substr(boundaryPos + 9);
		if (boundary.empty()) {
			LOG_ERROR("Invalid boundary in Content-Type header: " + contentType);
			_response = buildDefaultResponse(Http::BAD_REQUEST);
			return true;
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
				_response = buildDefaultResponse(Http::BAD_REQUEST);
				return true;
			}
			if (contentTypeFile.empty()) {
				LOG_ERROR("Missing Content-Type header in part");
				_response = buildDefaultResponse(Http::BAD_REQUEST);
				return true;
			}
			if (contentDisposition.find("filename=") != std::string::npos) {
				LOG_DEBUG("Handling file upload");
				if (setFileNameAndBody(part, contentDisposition)) {
					return true;  // TODO maybe break
				}
				break;
			}
		}
	}
	LOG_DEBUG("Handling file upload");
	return handleFileUpload();
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

bool RequestHandler::handleFileUpload() {
	const int fd = open(_fileName.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd < 0) {
		LOG_ERROR("Failed to open file descriptor for: " + _fileName + " with error: " + strerror(errno));
		_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
		return true;
	}

	pollfd pfd{};
	pfd.fd = fd;
	pfd.events = POLLOUT;

	const int pollret = poll(&pfd, 1, DEFAULT_POLL_TIMEOUT);
	if (pollret < 0) {
		LOG_ERROR("Poll error for file: " + _fileName + " with error: " + strerror(errno));
		close(fd);
		_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
		return true;
	}
	if (pollret == 0) {
		LOG_WARN("Poll timeout for file: " + _fileName);
		close(fd);
		_response = buildDefaultResponse(Http::REQUEST_TIMEOUT);
		return true;
	}
	if (!(pfd.revents & POLLOUT)) {
		LOG_WARN("Unexpected poll revents for file: " + _fileName + " revents: " + std::to_string(pfd.revents));
		close(fd);
		_response = buildDefaultResponse(Http::REQUEST_TIMEOUT);
		return true;
	}

	const ssize_t written = write(fd, _request.getBody().c_str(), std::min(POST_WRITE_SIZE, _request.getBody().size()));
	if (written < 0) {
		LOG_ERROR("Failed to write to file: " + _fileName + " with error: " + strerror(errno));
		close(fd);
		_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
		return true;
	}

	_bytesWrittenToFile += written;	 // Save how much is written already
	_request.getBodyRef().erase(0, written);	 // Remove the written part from body

	close(fd);

	if (_request.getBody().size() != 0) {
		return false;
	}

	LOG_INFO("File uploaded successfully: " + _fileName);
	_response = buildDefaultResponse(Http::CREATED);
	return true;
}

bool RequestHandler::setFileNameAndBody(const std::string &part, const std::string &contentDisposition) {
	std::string filename;
	std::size_t filenamePos = contentDisposition.find("filename=");
	if (filenamePos != std::string::npos) {
		filename = contentDisposition.substr(filenamePos + 10);
		filename = filename.substr(0, filename.find('\"'));
	}
	if (filename.empty()) {
		LOG_ERROR("Invalid filename in Content-Disposition header: " + contentDisposition);
		_response = buildDefaultResponse(Http::BAD_REQUEST);
		return true;
	}
	std::string fileContent;
	std::size_t pos = part.find("\r\n\r\n");
	if (pos != std::string::npos)
		fileContent = part.substr(pos + 4);
	else {
		LOG_ERROR("Invalid part: " + part);
		_response = buildDefaultResponse(Http::BAD_REQUEST);
		return true;
	}
	if (!_matchedRoute.getUploadDir().empty()) {
		if (!_matchedRoute.getRoot().empty())
			filename = buildpath(_matchedRoute.getUploadDir(), filename, _matchedRoute.getRoot());
		else
			filename = buildpath(_matchedRoute.getUploadDir(), filename, _serverConfig.getRoot());
	} else {
		if (!_serverConfig.getUploadDir().empty())
			filename = buildpath(_serverConfig.getUploadDir(), filename, _serverConfig.getRoot());
		else {
			_response = buildDefaultResponse(Http::FORBIDDEN);
			return true;
		}
	}

	if (access(filename.c_str(), F_OK) == 0 && !_bytesWrittenToFile) {
		LOG_WARN("File already exists: " + filename);
		_response = buildDefaultResponse(Http::CONFLICT);
		return true;
	}

	_fileName = filename;
	_request.setBody(fileContent);
	return false;
}
