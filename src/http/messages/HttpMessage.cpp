/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMessage.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:37:17 by flfische          #+#    #+#             */
/*   Updated: 2024/10/05 15:02:41 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpMessage.hpp"

std::string HttpMessage::getHttpVersion() const { return _httpVersion; }

std::map<std::string, std::string> HttpMessage::getHeaders() const {
  return _headers;
}

std::string HttpMessage::getHeader(const std::string &key) const {
  if (_headers.find(key) != _headers.end()) {
    return _headers.at(key);
  }
  return nullptr;
}

std::string HttpMessage::getBody() const { return _body; }

void HttpMessage::setHttpVersion(const std::string &httpVersion) {
  _httpVersion = httpVersion;
}

void HttpMessage::setHeaders(
    const std::map<std::string, std::string> &headers) {
  _headers = headers;
}

void HttpMessage::setBody(const std::string &body) { _body = body; }

/**
 * @brief Add a header to the message. Overwrites the value if the key already
 * exists.
 * @param key The header key
 * @param value The header value
 */
void HttpMessage::addHeader(const std::string &key, const std::string &value) {
  _headers[key] = value;
}

/**
 * @brief Add a header to the message. Only adds the header if the key does not
 * already exist.
 * @param key The header key
 * @param value The header value
 */
void HttpMessage::addHeaderIfNew(const std::string &key,
                                 const std::string &value) {
  if (_headers.find(key) == _headers.end()) {
    _headers[key] = value;
  }
}
