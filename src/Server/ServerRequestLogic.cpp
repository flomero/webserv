/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerRequestLogic.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 15:43:23 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/15 16:49:28 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Logger.hpp"

/**
 * @brief Main logic:
 * @brief   - Extract the location from the URI
 * @brief     -> Check if the location has cgi defined: handle cgi logic
 * @brief   - Build the server path to the ressource
 * @brief   - Verify it's type by trying to open it (can be saved in the request)
 * @brief     -> Handle the request depending on it's type GET/POST/DELETE
 *
 * @param request
 */
void Server::handleRequest(HttpRequest& request) {
	// Extract the location from the URI
	LOG_INFO("Extracting location path from URI");
	std::string uri = request.getRequestUri();

	size_t pathStart = uri.find_first_of('/');
	std::string location("/"); // Defaults to "/" if empty
	if (pathStart != uri.npos)
		location = uri.substr(pathStart, uri.size());
	request.setServerSidePath(_root + location);

	LOG_DEBUG("  |- uri:                " + uri);
	LOG_DEBUG("  |- location:           " + location);
	LOG_DEBUG("  |- server side path:   " + request.getServerSidePath() + "\n");


	// Match to the server's possible locations
	LOG_INFO("Getting best match for the corresponding location path");
	// Track the best match
	Route bestMatch;
	size_t longestMatchLength = 0;
	for (auto route: _routes) {
		// Check if the current route is a prefix of the path
		if (location.find(route.getPath()) == 0) { // Route is a prefix of the path
			size_t routeLength = route.getPath().size();

			// Select this route if it's the longest match so far
			if (routeLength > longestMatchLength) {
				bestMatch = route;
				longestMatchLength = routeLength;
			}
		}
	}

	LOG_DEBUG("  |- location:     " + location);
	LOG_DEBUG("  |- best match:   " + bestMatch.getPath() + "\n");
}
