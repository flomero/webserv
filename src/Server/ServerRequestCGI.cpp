/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerRequestCGI.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 16:54:21 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/28 15:17:56 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "ServerConfig.hpp"

/**
 * @brief Main logic:
 * @brief   - If file => Extract file extension
 * @brief     -> If supported => execv w/ path from config on file
 * @brief   - If folder
 * @brief     -> Check for autoindex => If supported execute
 *
 * @param request
 * @param route
 */
void ServerConfig::handleRequestCGI(HttpRequest& request, Route& route) {
	LOG_INFO("Entered handleRequestCGI");

	if (request.getIsFile()) {
		LOG_INFO("Handling file CGI");

		// Checks if this extensions has to be handled by a CGI
		LOG_INFO("Checks if this extension has to be handled by a CGI");

		if (route.getCgiHandlers().at(request.getRessourceExtension()).empty())
			LOG_DEBUG("  |- No CGI handlers found for extension:  " + request.getRessourceExtension() + "\n");
		else {
			LOG_DEBUG("  |- Found:            " + request.getRessourceExtension());
			LOG_DEBUG("  |- Executable path:  " + route.getCgiHandlers().at(request.getRessourceExtension()) + "\n");
			handleRequestCGIExecution(request, route);
		}

	} else {
		LOG_INFO("Handling directory CGI");
	}
}
