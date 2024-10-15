/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerPostRequest.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/11 14:43:11 by flfische          #+#    #+#             */
/*   Updated: 2024/10/15 14:58:20 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int Server::handlePostRequest(HttpRequest &request) {
	std::string contentType = request.getHeader("Content-Type");
	if (contentType == "application/x-www-form-urlencoded") {
		LOG_INFO("application/x-www-form-urlencoded");
		// TODO: not sure if needed
	} else if (contentType == "multipart/form-data")
		return handlePostMultipart(request);

	LOG_ERROR("Unsupported content type: " + contentType);
	return 415;
}

int Server::handlePostMultipart(HttpRequest &request) {
	std::string contentType = request.getHeader("Content-Type");
	std::size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos) {
		LOG_ERROR("Invalid Content-Type header: " + contentType);
		return Http::BAD_REQUEST;
	}
	std::string boundary = contentType.substr(boundaryPos + 9);
	if (boundary.empty()) {
		LOG_ERROR("Invalid boundary in Content-Type header: " + contentType);
		return Http::BAD_REQUEST;
	}
	std::string boundaryDelimiter = "--" + boundary;
	std::string boundaryEnd = boundaryDelimiter + "--";
	std::size_t pos = 0;

	std::string body = request.getBody();
	while ((pos = body.find(boundaryDelimiter, pos)) != std::string::npos) {
		pos += boundaryDelimiter.length();
		if (pos == body.length())
			break;
		std::size_t endPos = body.find(boundaryDelimiter, pos);
		std::string part = body.substr(pos, endPos - pos);
		pos = endPos;

		std::istringstream partStream(part);
		std::string line;
		std::string contentDisposition;
		std::string contentType;

		while (std::getline(partStream, line) && !line.empty()) {
			if (line.find("Content-Disposition") == 0)
				contentDisposition = line;
			else if (line.find("Content-Type") == 0)
				contentType = line;
		}
		if (contentDisposition.empty()) {
			LOG_ERROR("Missing Content-Disposition header in part");
			return Http::BAD_REQUEST;
		}
		if (contentType.empty()) {
			LOG_ERROR("Missing Content-Type header in part");
			return Http::BAD_REQUEST;
		}
		if (contentDisposition.find("filename=") != std::string::npos)
			return handleFileUpload(part, contentDisposition);
		else {
			// TODO: handle form fields - not sure if needed
			LOG_INFO("Form field: " + part);
		}
	}
	return Http::OK;
}

int Server::handleFileUpload(const std::string &part,
							 const std::string &contentDisposition) {
	std::string filename;
	std::size_t filenamePos = contentDisposition.find("filename=");
	if (filenamePos != std::string::npos) {
		filename = contentDisposition.substr(filenamePos + 9);
		filename = filename.substr(0, filename.find("\""));
	}
	if (filename.empty()) {
		LOG_ERROR("Invalid filename in Content-Disposition header: " +
				  contentDisposition);
		return Http::BAD_REQUEST;
	}
	std::string fileContent;
	std::size_t pos = part.find("\r\n\r\n");
	if (pos != std::string::npos)
		fileContent = part.substr(pos + 4);
	else {
		LOG_ERROR("Invalid part: " + part);
		return Http::BAD_REQUEST;
	}
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		LOG_ERROR("Failed to open file: " + filename);
		return Http::INTERNAL_SERVER_ERROR;
	}
	file.write(fileContent.c_str(), fileContent.length());
	file.close();
	LOG_INFO("File uploaded: " + filename);
	return Http::OK;
}
