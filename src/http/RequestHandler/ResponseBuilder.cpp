/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 19:57:06 by flfische          #+#    #+#             */
/*   Updated: 2024/11/03 16:19:07 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "ServerConfig.hpp"

/**
 * @brief Builds a default response for a given status code
 * @param code The status code
 * @return The response - will use the defined error page if available as body
 */
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

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
	if (errorPage.has_value()) {
		//  TODO: check if this is correct
		// build path to error page
		std::string path = "." + _serverConfig.getRoot();
		if (path.back() != '/') {
			path += '/';
		}
		path += errorPage.value();
		LOG_DEBUG("Error page path: " + path);
		int fd = open(path.c_str(), O_RDONLY);
		if (fd != -1) {
			struct pollfd pfd;
			pfd.fd = fd;
			pfd.events = POLLIN;
			int timeout = 5000;	 // TODO: make this configurable

			int ret = poll(&pfd, 1, timeout);
			if (ret > 0 && (pfd.revents & POLLIN)) {
				std::ifstream file(path);
				if (file.is_open()) {
					std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					response.setBody(content);
					file.close();
				}
			}
			close(fd);
		} else {
			LOG_ERROR("Failed to open error page: " + std::string(strerror(errno)));
		}
	}

	if (response.getBody().empty()) {
		std::ostringstream body;
		body << "<html><head><title>404</title><meta charset=\"utf-8\">"
			 << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
			 << "</head><body><h1>Error " << code << ": " << Http::getStatusMessage(code) << "</h1>"
			 << "<img src=\"https://httpgoats.com/" << code << ".jpg\" alt=\"Goat\">"
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
