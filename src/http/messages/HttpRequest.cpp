/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:19:37 by flfische          #+#    #+#             */
/*   Updated: 2024/10/04 17:51:22 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

#include <sstream>

HttpRequest::HttpRequest(const std::string &rawRequest) {
  std::istringstream requestStream(rawRequest);
  std::string line;
  std::getline(requestStream, line);
  std::istringstream lineStream(line);
  lineStream >> method >> requestUri >> httpVersion;
  if (method.empty() || requestUri.empty() || httpVersion.empty()) {
    throw InvalidRequest();
  }
  while (std::getline(requestStream, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (line.empty()) {
      break;
    }
    std::string key, value;
    std::istringstream headerStream(line);
    std::getline(headerStream, key, ':');
    if (headerStream.peek() == ' ') headerStream.get();
    std::getline(headerStream, value);
    addHeader(key, value);
  }
  // Read body
  std::string body;
  std::getline(requestStream, body, '\0');
  setBody(body);
}
std::string HttpRequest::getMethod() const { return method; }

std::string HttpRequest::getRequestUri() const { return requestUri; }

void HttpRequest::setMethod(const std::string &method) {
  this->method = method;
}

void HttpRequest::setRequestUri(const std::string &requestUri) {
  this->requestUri = requestUri;
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