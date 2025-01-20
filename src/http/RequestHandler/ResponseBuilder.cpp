/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 19:57:06 by flfische          #+#    #+#             */
/*   Updated: 2025/01/19 11:06:36 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include "webserv.hpp"

/**
 * @brief Builds a default response for a given status code
 * @param code The status code
 * @return The response - will use the defined error page if available as body
 */
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <optional>
#include <string>

HttpResponse RequestHandler::buildDefaultResponse(Http::Status code, std::optional<HttpRequest> request) {
	HttpResponse response(code);

	if (request.has_value()) {
		_request = request.value();
	}

	// Check if there's a configured error page for this status code
	std::optional<std::string> errorPage = _serverConfig.getErrorPage(code);
	try {
		if (errorPage.has_value()) {
			std::string path = "." + _serverConfig.getRoot() + "/" + errorPage.value();
			std::filesystem::path fsPath(path);
			LOG_DEBUG("Error page path: " + path);

			if (std::ifstream file(fsPath); file.is_open()) {
				std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
				response.setBody(content);
				file.close();
			} else {
				LOG_ERROR("Failed to open error page: " + std::string(strerror(errno)));
			}
		}
	} catch (const std::exception &e) {
		LOG_ERROR("Error while getting custom error page: " + std::string(e.what()));
	}

	if (response.getBody().empty()) {
		std::ostringstream body;
		body << "<html><head><title>" << code << "</title><meta charset=\"utf-8\">"
			 << R"(<meta name="viewport" content="width=device-width, initial-scale=1">)"
			 << "</head><body><h1>Error " << code << ": " << getStatusMessage(code) << "</h1>"
			 << "<img src=\"https://httpgoats.com/" << code << R"(.jpg" alt="Goat">)"
			 << "</body><style>*{font-family:Arial,sans-serif;--background:#f2f2f2;--color:#030303;}"
			 << "@media (prefers-color-scheme: dark){*{--background:#030303;--color:#f2f2f2;}}"
			 << "body{display:flex;justify-content:center;align-items:center;height:100vh;margin:0;"
			 << "flex-direction:column;background:var(--background);color:var(--color);}"
			 << "h1{font-size:clamp(2rem,5vw,3rem);}"
			 << "img{width:100%;max-width:500px;}</style></html>";

		response.setBody(body.str());
		response.addHeader("Content-Type", "text/html");
	}

	response.addHeader("Content-Length", std::to_string(response.getBody().size()));

	if (_request.hasHeader("Connection")) {
		response.addHeader("Connection", _request.getHeader("Connection"));
	}
	if (code >= 500) {
		response.addHeader("Connection", "close");
	}

	return response;
}
