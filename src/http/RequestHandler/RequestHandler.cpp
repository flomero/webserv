/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 15:43:23 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/31 16:11:03 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

#include "Logger.hpp"
#include "ServerConfig.hpp"

/**
 * @brief Construct a new Request Handler:: Request Handler object
 *
 * @param request
 * @param serverConfig
 */
RequestHandler::RequestHandler(ServerConfig& serverConfig) : _serverConfig(serverConfig) {
	LOG_INFO("RequestHandler created");
}

/**
 * @brief get closest matching route and save it in _matchedRoute
 */
void RequestHandler::findMatchingRoute() {
	// Match to the server's possible locations
	LOG_INFO("Getting best match for the corresponding location path");
	// Track the best match
	Route matchedRoute;
	size_t longestMatchLength = 0;
	for (auto route : _serverConfig.getRoutes()) {
		// Check if the current route is a prefix of the path
		if (_request.getLocation().find(route.getPath()) == 0) {  // Route is a prefix of the path
			size_t routeLength = route.getPath().size();

			// Select this route if it's the longest match so far
			if (routeLength > longestMatchLength) {
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
 * @brief   - Verify it's type by trying to open it (can be saved in the
 * request)
 * @brief     -> Handle the request depending on it's type GET/POST/DELETE
 *
 * @param request
 */
HttpResponse RequestHandler::handleRequest(HttpRequest& request) {
	_request = request;
	_request.setServerSidePath("." + _serverConfig.getRoot() + request.getLocation());

	LOG_DEBUG("  |- uri:                     " + _request.getRequestUri());
	LOG_DEBUG("  |- location:                " + _request.getLocation());
	LOG_DEBUG("  |- server side path:        " + _request.getServerSidePath());
	std::filesystem::path serverSidePath(_request.getServerSidePath());
	LOG_DEBUG("  |- filesystem::path:        " + serverSidePath.generic_string() + "\n");

	// Find the best matching route
	findMatchingRoute();

	// Check ressource existence
	if (_request.getMethod() != "POST" ||
		_matchedRoute.getCgiHandlers().size() > 0) {  // Check only if not POST or POST w/ CGI
		LOG_INFO("Checking ressource existence");
		if (!std::filesystem::exists(serverSidePath))
			return HttpResponse(500);  // TODO: Early return if ressource doesn't exist (TODO: any error
									   // code for this ?)
		_request.setIsFile(std::filesystem::is_regular_file(serverSidePath));

		LOG_DEBUG("  |- Ressource exists");
		LOG_DEBUG((_request.getIsFile()) ? "  |- Ressource is a file\n" : "  |- Ressource is a directory\n");

		if (_request.getIsFile()) {
			// Extracting file extension
			LOG_INFO("Extracting ressource extensions");
			size_t fileStart = _request.getServerSidePath().find_last_of('/');
			LOG_DEBUG("  |- Ressource from trailing '/':  " +
					  _request.getServerSidePath().substr(fileStart + 1, _request.getServerSidePath().back()));
			std::string filename =
				_request.getServerSidePath().substr(fileStart + 1, _request.getServerSidePath().back());
			size_t extensionStart = filename.find_first_of(".");
			LOG_DEBUG("  |- Extension:                    " + filename.substr(extensionStart, filename.back()) + "\n");
			_request.setRessourceExtension(filename.substr(extensionStart, filename.back()));
		}
	}

	// Check for CGI on the Route
	LOG_INFO("Checking for route's informations: CGI");
	if (_matchedRoute.getCgiHandlers().size() > 0)
		handleRequestCGI(_matchedRoute);
	else
		LOG_INFO("Did not enter handleRequestCGI");

	return _response;
}
