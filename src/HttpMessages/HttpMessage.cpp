/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMessage.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:37:17 by flfische          #+#    #+#             */
/*   Updated: 2024/10/04 16:38:29 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpMessage.hpp"

std::string HttpMessage::getHttpVersion() const { return httpVersion; }

std::map<std::string, std::string> HttpMessage::getHeaders() const {
  return headers;
}

std::string HttpMessage::getBody() const { return body; }

void HttpMessage::setHttpVersion(const std::string &httpVersion) {
  this->httpVersion = httpVersion;
}

void HttpMessage::setHeaders(const std::map<std::string, std::string> &headers) {
  this->headers = headers;
}

void HttpMessage::setBody(const std::string &body) { this->body = body; }

void HttpMessage::addHeader(const std::string &key, const std::string &value) {
  headers[key] = value;
}
