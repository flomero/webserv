/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMessage.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:37:17 by flfische          #+#    #+#             */
/*   Updated: 2024/11/03 16:18:36 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpMessage.hpp"

#pragma region Getters

std::string HttpMessage::getHttpVersion() const { return _httpVersion; }

std::map<std::string, std::string> HttpMessage::getHeaders() const { return _headers; }

std::string HttpMessage::getHeader(const std::string &key) const {
	if (_headers.find(key) != _headers.end()) {
		return _headers.at(key);
	}
	return "";
}

std::string HttpMessage::getBody() const { return _body; }

std::string &HttpMessage::getBodyRef() { return _body; }

bool HttpMessage::hasHeader(const std::string &key) const { return _headers.find(key) != _headers.end(); }

#pragma endregion

#pragma region Setters

void HttpMessage::setHttpVersion(const std::string &httpVersion) { _httpVersion = httpVersion; }

void HttpMessage::setHeaders(const std::map<std::string, std::string> &headers) { _headers = headers; }

void HttpMessage::setBody(const std::string &body) { _body = body; }

#pragma endregion

#pragma region Header manipulation

/**
 * @brief Add a header to the message. Overwrites the value if the key already
 * exists.
 * @param key The header key
 * @param value The header value
 */
void HttpMessage::addHeader(const std::string &key, const std::string &value) { _headers[key] = value; }

/**
 * @brief Add a header to the message. Only adds the header if the key does not
 * already exist.
 * @param key The header key
 * @param value The header value
 */
void HttpMessage::addHeaderIfNew(const std::string &key, const std::string &value) {
	if (_headers.find(key) == _headers.end()) {
		_headers[key] = value;
	}
}

#pragma endregion

#pragma region Body manipulation

/**
 * @brief Append data to the message body
 * @param newData The data to append
 */
void HttpMessage::appendToBody(const std::string &newData) { _body += newData; }

#pragma endregion
