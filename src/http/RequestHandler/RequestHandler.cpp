/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 15:43:23 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/06 14:51:17 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

#include "Logger.hpp"
#include "ServerConfig.hpp"

/**
 * @brief Construct a new Request Handler:: Request Handler object
 *
 * @param serverConfig
 */
RequestHandler::RequestHandler(ServerConfig& serverConfig) : _serverConfig(serverConfig) {
	LOG_INFO("RequestHandler created");
}

#pragma region Getters

ServerConfig& RequestHandler::getConfig() const { return _serverConfig; }

#pragma endregion

/**
 * @brief get the closest matching route and save it in _matchedRoute
 */
void RequestHandler::findMatchingRoute() {
	// Match to the server's possible locations
	LOG_INFO("Getting best match for the corresponding location path");
	// Track the best match
	Route matchedRoute;
	size_t longestMatchLength = 0;
	for (const auto& route : _serverConfig.getRoutes()) {
		// Check if the current route is a prefix of the path
		if (_request.getLocation().find(route.getPath()) == 0) {  // Route is a prefix of the path

			// Select this route if it's the longest match so far
			if (const size_t routeLength = route.getPath().size(); routeLength > longestMatchLength) {
				matchedRoute = route;
				longestMatchLength = routeLength;
			}
		}
	}
	_matchedRoute = matchedRoute;
	LOG_DEBUG("  |- best match:   " + _matchedRoute.getPath() + "\n");
}

/**
 * @brief Main logic:
 * @brief     -> Check if the location has cgi defined: handle cgi logic
 * @brief   - Build the server path to the ressource
 * @brief   - Verify its type by trying to open it (can be saved in the
 * request)
 * @brief     -> Handle the request depending on its type GET/POST/DELETE
 *
 * @param request
 */
HttpResponse RequestHandler::handleRequest(const HttpRequest& request) {
	_request = request;
	_response = HttpResponse();
	_request.setServerSidePath("." + _serverConfig.getRoot() + request.getLocation());

	LOG_DEBUG("  |- uri:                     " + _request.getRequestUri());
	LOG_DEBUG("  |- location:                " + _request.getLocation());
	LOG_DEBUG("  |- server side path:        " + _request.getServerSidePath());
	const std::filesystem::path serverSidePath(_request.getServerSidePath());
	LOG_DEBUG("  |- filesystem::path:        " + serverSidePath.generic_string() + "\n");

	// Find the best matching route
	findMatchingRoute();

	// check if method is allowed
	if (std::find(_matchedRoute.getMethods().begin(), _matchedRoute.getMethods().end(), _request.getMethod()) ==
		_matchedRoute.getMethods().end()) {
		LOG_WARN("Method not allowed");
		return buildDefaultResponse(Http::METHOD_NOT_ALLOWED);
	}

	// Check resource existence
	if (_request.getMethod() != "POST" ||
		!_matchedRoute.getCgiHandlers().empty()) {	// Check only if not POST or POST w/ CGI
		LOG_INFO("Checking resource existence");
		if (!exists(serverSidePath))
			return buildDefaultResponse(Http::NOT_FOUND);
		_request.setIsFile(is_regular_file(serverSidePath));

		LOG_DEBUG("  |- Resource exists");
		LOG_DEBUG(_request.getIsFile() ? "  |- Resource is a file\n" : "  |- Resource is a directory\n");

		if (_request.getIsFile()) {
			// Extracting file extension
			LOG_INFO("Extracting resource extensions");
			const size_t fileStart = _request.getServerSidePath().find_last_of('/');
			LOG_DEBUG("  |- Resource from trailing '/':  " +
					  _request.getServerSidePath().substr(fileStart + 1, _request.getServerSidePath().back()));
			const std::string filename =
				_request.getServerSidePath().substr(fileStart + 1, _request.getServerSidePath().back());
			const size_t extensionStart = filename.find_first_of('.');
			LOG_DEBUG("  |- Extension:                    " + filename.substr(extensionStart, filename.back()) + "\n");
			_request.setResourceExtension(filename.substr(extensionStart, filename.back()));
		}
	}

	// Check for CGI on the Route
	LOG_INFO("Checking for route's information's: CGI");
	if (!_matchedRoute.getCgiHandlers().empty()) {
		handleRequestCGI(_matchedRoute);
		if (_cgiExecuted)
			return _response;
		LOG_DEBUG("  |- No CGI handlers found for extension:  " + _request.getResourceExtension());
	}
	if (_request.getMethod() == "GET")
		_response = handleGetRequest();
	else if (_request.getMethod() == "POST")
		_response = handlePostRequest();
	else if (_request.getMethod() == "DELETE")
		_response = handleDeleteRequest();

	if (_request.getHttpVersion() == "HTTP/1.0")
		_response.setHttpVersion("HTTP/1.0");
	if (_request.getHeader("Connection") == "close")
		_response.addHeader("Connection", "close");
	else if (_request.getHeader("Connection") == "keep-alive")
		_response.addHeader("Connection", "keep-alive");
	_response.setDefaultHeaders();
	return _response;
}
