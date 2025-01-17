/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 15:43:23 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/17 16:38:57 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

#include <algorithm>
#include <string>
#include <vector>

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

void RequestHandler::setConfig(const ServerConfig& server_config) const { _serverConfig = server_config; }

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

	if (!_matchedRoute.getRoot().empty()) {
		_request.setServerSidePath("." + _matchedRoute.getRoot() + "/" +
								   _request.getLocation().erase(0, longestMatchLength));
	} else
		_request.setServerSidePath("." + _serverConfig.getRoot() + "/" + _request.getLocation());

	// if matches directly to a route, check for index file in the directory and change if applicable
	if (_request.getLocation().back() == '/') {
		std::string indexFile;
		if (_matchedRoute.getIndex() != "") {
			indexFile = _matchedRoute.getIndex();
		} else {
			indexFile = _serverConfig.getIndex();
		}
		if (indexFile.empty()) {
			return;
		}
		std::string indexPath = _request.getServerSidePath();
		if (indexPath.back() != '/') {
			indexPath += '/';
		}
		if (indexFile.front() == '/') {
			indexFile = indexFile.substr(1);
		}
		indexPath += indexFile;
		const std::filesystem::path path(indexPath);
		if (exists(path)) {
			_request.setServerSidePath(path);
			_request.setIsFile(true);
		}
	}
	LOG_DEBUG("  |- server side path:  " + _request.getServerSidePath() + "\n");
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
bool RequestHandler::handleRequest(const HttpRequest& request) {
	_request = request;

	// LOG_ERROR("request location: " + request.getLocation());
	// _request.setServerSidePath("." + _serverConfig.getRoot() + request.getLocation());
	_cgiExecuted = false;

	if (!_parsingDone) {
		LOG_DEBUG("  |- uri:                     " + _request.getRequestUri());
		LOG_DEBUG("  |- location:                " + _request.getLocation());
		LOG_DEBUG("  |- server side path:        " + _request.getServerSidePath());

		// Find the best matching route
		findMatchingRoute();

		const std::filesystem::path serverSidePath(_request.getServerSidePath());
		LOG_DEBUG("  |- filesystem::path:        " + serverSidePath.generic_string() + "\n");

		if (_matchedRoute.getCode() != 0) {
			LOG_INFO("Route has a return directive.");
			_response = handleRedirectRequest();
			return true;
		}

		// check if method is allowed
		if (std::find(_matchedRoute.getMethods().begin(), _matchedRoute.getMethods().end(), _request.getMethod()) ==
			_matchedRoute.getMethods().end()) {
			LOG_WARN("Method not allowed");

			_response = buildDefaultResponse(Http::METHOD_NOT_ALLOWED);
			return true;
		}

		// Check resource existence
		if (_request.getMethod() != "POST" ||
			!_matchedRoute.getCgiHandlers().empty()) {	// Check only if not POST or POST w/ CGI
			LOG_INFO("Checking resource existence");
			if (!exists(serverSidePath)) {
				_response = buildDefaultResponse(Http::NOT_FOUND);
				return true;
			}
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
				LOG_DEBUG("  |- Extension:                    " + filename.substr(extensionStart, filename.back()) +
						  "\n");
				_request.setResourceExtension(filename.substr(extensionStart, filename.back()));
			}
		}
		_parsingDone = true;
	}

	// Check for CGI on the Route
	LOG_INFO("Checking for route's information's: CGI");
	if (!_matchedRoute.getCgiHandlers().empty()) {
		handleRequestCGI(_matchedRoute);
		if (_cgiExecuted) {
			if (!_request.getHeader("Connection").empty())
				_response.addHeader("Connection", _request.getHeader("Connection"));
			_response.setDefaultHeaders();
			return true;
		}
		LOG_DEBUG("  |- No CGI handlers found for extension:  " + _request.getResourceExtension());
	}

	bool isFinished = false;

	if (_request.getMethod() == "GET")
		isFinished = handleGetRequest();
	else if (_request.getMethod() == "POST")
		isFinished = handlePostRequest();
	else if (_request.getMethod() == "DELETE") {
		handleDeleteRequest();
		isFinished = true;
	}

	if (isFinished) {
		if (_request.getHttpVersion() == "HTTP/1.0")
			_response.setHttpVersion("HTTP/1.0");
		if (!_request.getHeader("Connection").empty())
			_response.addHeader("Connection", _request.getHeader("Connection"));
		_response.setDefaultHeaders();
	}
	return isFinished;
}

HttpResponse RequestHandler::getResponse() {
	_bytesReadFromFile = 0;
	_response = HttpResponse();
	_parsingDone = false;

	return _response;
}
