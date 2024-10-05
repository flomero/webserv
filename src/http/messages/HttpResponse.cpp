/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/05 13:53:24 by flfische          #+#    #+#             */
/*   Updated: 2024/10/05 15:03:28 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

#include <chrono>
#include <string>

#include "HttpStatus.hpp"

HttpResponse::HttpResponse(int status) : _status(status) {
  setDefaultHeaders();
}

void HttpResponse::SetStatus(int status) { _status = status; }

int HttpResponse::GetStatus() const { return _status; }

std::string getCurrentDate() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::string date = std::ctime(&now_c);
  date.pop_back();
  date += " GMT";
  return date;
}

void HttpResponse::setDefaultHeaders() {
  addHeaderIfNew("Server", "Webserv");
  addHeaderIfNew("Content-Type", "text/html");
  addHeaderIfNew("Connection", "close");
  addHeaderIfNew("Content-Length", std::to_string(_body.length()));
  addHeaderIfNew("Date", getCurrentDate());
  // ToDo: Add more headers
}

std::string HttpResponse::ToString() const {
  std::string str = _httpVersion + " " + std::to_string(_status) + " " +
                    getStatusMessage(_status) + "\r\n";
  for (const auto &[key, value] : _headers) {
    str += key + ": " + value + "\r\n";
  }
  str += "\r\n" + _body;
  return str;
}
