/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/11 14:43:11 by flfische          #+#    #+#             */
/*   Updated: 2025/01/17 17:01:15 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cerrno>
#include <sys/stat.h>

#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "ServerConfig.hpp"

HttpResponse RequestHandler::handlePostRequest() {
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
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		LOG_ERROR("Failed to open file: " + filename);
		return buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
	}
	file.write(fileContent.c_str(), fileContent.size());
	file.close();
	LOG_INFO("File uploaded: " + filename);
	return buildDefaultResponse(Http::CREATED);
}
