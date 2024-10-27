/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:19:37 by flfische          #+#    #+#             */
/*   Updated: 2024/10/21 16:16:20 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

#include <sstream>

const std::vector<std::string> HttpRequest::_supportedMethods = {"GET", "POST",
																 "DELETE"};

// used to differentiate between 501 and 400
const std::vector<std::string> HttpRequest::_unsupportedMethods = {
	"HEAD", "PUT", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

const std::vector<std::string> HttpRequest::_supportedVersions = {"HTTP/1.0",
																  "HTTP/1.1"};

HttpRequest::HttpRequest(const std::string &rawRequest) {
	std::istringstream requestStream(rawRequest);
	std::string line;
	std::getline(requestStream, line);
	std::istringstream lineStream(line);
	lineStream >> _method >> _requestUri >> _httpVersion;
	if (_method.empty() || _requestUri.empty() || _httpVersion.empty()) {
		throw BadRequest();
	}
	validate();
	while (std::getline(requestStream, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			break;
		std::istringstream headerStream(line);
		std::string key, value;
		std::getline(headerStream, key, ':');
		headerStream.ignore(1, ' ');
		std::getline(headerStream, value);
		addHeader(key, value);
	}
	if (getHeader("Transfer-Encoding") == "chunked") {
		parseChunkedBody(requestStream);
	} else {
		auto contentLengthHeader = getHeader("Content-Length");
		if (!contentLengthHeader.empty()) {
			std::size_t contentLength = std::stoul(contentLengthHeader);
			std::string body(contentLength, '\0');
			requestStream.read(&body[0], contentLength);
			setBody(body);
		} else {
			setBody("");
		}
	}
}

void HttpRequest::validate() const {
	if (std::find(_supportedMethods.begin(), _supportedMethods.end(),
				  _method) == _supportedMethods.end()) {
		if (std::find(_unsupportedMethods.begin(), _unsupportedMethods.end(),
					  _method) != _unsupportedMethods.end()) {
			throw NotImplemented();
		}
		throw BadRequest();
	}
	if (std::find(_supportedVersions.begin(), _supportedVersions.end(),
				  _httpVersion) == _supportedVersions.end()) {
		throw InvalidVersion();
	}
}

void HttpRequest::parseChunkedBody(std::istringstream &requestStream) {
	std::vector<char> body;
	std::string line;

	while (true) {
		if (!std::getline(requestStream, line))
			break;

		if (line.empty())
			continue;

		std::size_t chunkSize;
		try {
			chunkSize = std::stoul(line, nullptr, 16);
		} catch (const std::exception &e) {
			throw BadRequest();
		}

		if (chunkSize == 0)
			break;

		std::string chunk(chunkSize, '\0');
		requestStream.read(&chunk[0], chunkSize);

		if (requestStream.gcount() < static_cast<std::streamsize>(chunkSize))
			break;

		body.insert(body.end(), chunk.begin(), chunk.end());
		requestStream.ignore(2);
	}

	setBody(std::string(body.begin(), body.end()));
}

std::string HttpRequest::getMethod() const { return _method; }

std::string HttpRequest::getRequestUri() const { return _requestUri; }

std::string HttpRequest::getServerSidePath() const { return _serverSidePath; }

bool HttpRequest::getIsFile() const { return _isFile; }

std::string HttpRequest::getRessourceExtension() const {
	return _ressourceExtension;
}

std::string HttpRequest::getQueryString() const { return _queryString; }
int HttpRequest::getRequestLength() const {
	return _rawRequest.length() + getBody().length(); //Todo: check if this is correct
}

void HttpRequest::setMethod(const std::string &method) { _method = method; }

void HttpRequest::setRequestUri(const std::string &requestUri) {
	_requestUri = requestUri;
}

void HttpRequest::setServerSidePath(const std::string &serverSidePath) {
	_serverSidePath = serverSidePath;
}

void HttpRequest::setIsFile(bool isFile) { _isFile = isFile; }

void HttpRequest::setRessourceExtension(const std::string &ressourceExtension) {
	_ressourceExtension = ressourceExtension;
}

void HttpRequest::setQueryString(const std::string &queryString) {
	_queryString = queryString;
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &request) {
	os << request.getMethod() << " " << request.getRequestUri() << " "
	   << request.getHttpVersion() << "\r\n";
	for (const auto &header : request.getHeaders()) {
		os << header.first << ": " << header.second << "\r\n";
	}
	os << "\r\n" << request.getBody();
	return os;
}
