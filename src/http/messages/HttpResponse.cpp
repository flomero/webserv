/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/05 13:53:24 by flfische          #+#    #+#             */
/*   Updated: 2025/01/16 14:16:09 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

#include <chrono>
#include <string>

#include "HttpStatus.hpp"
#include "webserv.hpp"

HttpResponse::HttpResponse(const Http::Status status) : _status(status) { setDefaultHeaders(); }

HttpResponse::HttpResponse(int status) : _status(static_cast<Http::Status>(status)) { setDefaultHeaders(); }

HttpResponse::HttpResponse(const std::string &rawResponse) {
	try {
		std::istringstream iss(rawResponse);
		std::string line;
		std::getline(iss, line);
		std::istringstream firstLine(line);
		std::string httpVersion;
		int status;
		firstLine >> httpVersion >> status;
		_status = static_cast<Http::Status>(status);
		while (std::getline(iss, line) && !line.empty()) {
			size_t pos = line.find(": ");
			if (pos != std::string::npos) {
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 2);
				_headers[key] = value;
			}
		}
		std::string body;
		while (std::getline(iss, line)) {
			body += line + "\n";
		}
		_body = body;

		if (_headers.find("Content-Length") == _headers.end()) {
			addHeaderIfNew("Content-Length", std::to_string(_body.size()));
		}
	} catch (...) {
		_status = Http::Status::INTERNAL_SERVER_ERROR;
		_body = "Internal Server Error";
		addHeaderIfNew("Content-Length", std::to_string(_body.size()));
	}
}

void HttpResponse::setStatus(const Http::Status status) { _status = status; }

Http::Status HttpResponse::getStatus() const { return _status; }

std::string getCurrentDate() {
	const auto now = std::chrono::system_clock::now();
	const std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::string date = std::ctime(&now_c);
	date.pop_back();
	date += " GMT";
	return date;
}

void HttpResponse::setDefaultHeaders() {
	addHeaderIfNew("Server", SERVER_NAME);
	addHeaderIfNew("Content-Type", "text/html");
	if (_httpVersion == "HTTP/1.1") {
		addHeaderIfNew("Connection", "keep-alive");
	} else {
		addHeaderIfNew("Connection", "close");
	}
	addHeaderIfNew("Content-Length", std::to_string(_body.length()));
	addHeaderIfNew("Date", getCurrentDate());
	// TODO: Add more headers???
}

std::string HttpResponse::toString() const {
	std::string str = _httpVersion + " " + std::to_string(_status) + " " + getStatusMessage(_status) + "\r\n";
	for (const auto &[key, value] : _headers) {
		str += key + ": ";
		str += value + "\r\n";
	}
	str += "\r\n" + _body;
	return str;
}
