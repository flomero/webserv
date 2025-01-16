/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:19:37 by flfische          #+#    #+#             */
/*   Updated: 2025/01/15 21:16:39 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

#include <sstream>

#include "Logger.hpp"

/**
 * @brief Constructs an HttpRequest object from a raw HTTP request (w/o body)
 * @param rawRequest The raw HTTP request
 */
HttpRequest::HttpRequest(const std::string &rawRequest) {
	_bodyType = BodyType::NO_BODY;
	std::istringstream requestStream(rawRequest);
	std::string line;
	std::getline(requestStream, line);
	std::istringstream lineStream(line);
	lineStream >> _method >> _requestUri >> _httpVersion;
	if (_method.empty() || _requestUri.empty() || _httpVersion.empty()) {
		throw BadRequest();
	}
	_decodeURL();
	_validateRequestLine();
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

	_initBodyType();

	_validateHeaders();
	_parseURI();
	setBody("");
}

void HttpRequest::_initBodyType() {
	if (getHeader("Transfer-Encoding") == "chunked") {
		_bodyType = BodyType::CHUNKED;
	} else if (!getHeader("Content-Length").empty()) {
		_bodyType = BodyType::CONTENT_LENGTH;
		_contentLength = std::stoul(getHeader("Content-Length"));
	} else {
		_bodyType = BodyType::NO_BODY;
	}
}

void HttpRequest::_validateRequestLine() const {
	if (_supportedMethods.find(_method) == _supportedMethods.end()) {
		if (_unsupportedMethods.find(_method) != _unsupportedMethods.end()) {
			LOG_WARN("Unsupported method: " + _method);
			throw NotImplemented();
		}
		LOG_WARN("Invalid method: " + _method);
		throw BadRequest();
	}
	if (_supportedVersions.find(_httpVersion) == _supportedVersions.end()) {
		LOG_WARN("Unsupported HTTP version: " + _httpVersion);
		throw InvalidVersion();
	}
}

void HttpRequest::_validateHeaders() const {
	// TODO: check if Host header is required
	// TODO: check if other stuff is required
	if (_method == "POST" && _bodyType == BodyType::NO_BODY) {
		throw BadRequest();
	}
}

void HttpRequest::_parseURI() {
	_location = "/";
	if (const std::size_t pathStart = _requestUri.find_first_of('/'); pathStart != std::string::npos) {
		_location = _requestUri.substr(pathStart, _requestUri.size());
	}
	LOG_DEBUG("  |- Location:            " + _location);
	if (size_t queryStart = _location.find_first_of('?'); queryStart != std::string::npos) {
		LOG_DEBUG("  |- Query string found:  " + _location);
		_queryString = _location.substr(queryStart + 1, _location.back());
		_location = _location.substr(0, queryStart);
		LOG_DEBUG("  |- Query string:        " + _queryString);
	}
}

void HttpRequest::_decodeURL() {
	auto it = _requestUri.begin();
	while (it != _requestUri.end()) {
		if (*it == '%') {
			try {
				std::string hex = _requestUri.substr(std::distance(_requestUri.begin(), it) + 1, 2);
				char c = std::stoi(hex, nullptr, 16);
				_requestUri.replace(it, it + 3, 1, c);
			} catch (const std::exception &e) {
				throw BadRequest();
			}
		} else {
			++it;
		}
	}
	LOG_DEBUG("  |- Decoded URI:         " + _requestUri);
}

#pragma region Getters

std::string HttpRequest::getMethod() const { return _method; }

std::string HttpRequest::getRequestUri() const { return _requestUri; }

std::string HttpRequest::getServerSidePath() const { return _serverSidePath; }

bool HttpRequest::getIsFile() const { return _isFile; }

std::string HttpRequest::getResourceExtension() const { return _resourceExtension; }

std::string HttpRequest::getQueryString() const { return _queryString; }

std::string HttpRequest::getLocation() const { return _location; }

#pragma endregion

#pragma region Setters

void HttpRequest::setMethod(const std::string &method) { _method = method; }

void HttpRequest::setRequestUri(const std::string &requestUri) { _requestUri = requestUri; }

void HttpRequest::setServerSidePath(const std::string &serverSidePath) { _serverSidePath = serverSidePath; }

void HttpRequest::setIsFile(const bool isFile) { _isFile = isFile; }

void HttpRequest::setResourceExtension(const std::string &resourceExtension) { _resourceExtension = resourceExtension; }

void HttpRequest::setQueryString(const std::string &queryString) { _queryString = queryString; }

void HttpRequest::setLocation(const std::string &location) { _location = location; }

#pragma endregion

#pragma region Print

std::ostream &operator<<(std::ostream &os, const HttpRequest &request) {
	os << request.getMethod() << " " << request.getRequestUri() << " " << request.getHttpVersion() << "\r\n";
	for (const auto &[key, val] : request.getHeaders()) {
		os << key << ": " << val << "\r\n";
	}
	os << "\r\n";
	os << request.getBody();
	return os;
}

#pragma endregion
