/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCGI.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 16:54:21 by lgreau            #+#    #+#             */
/*   Updated: 2024/12/10 16:54:01 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "Route.hpp"

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
void RequestHandler::handleRequestCGI(Route& route) {
	LOG_INFO("Entered handleRequestCGI");

	if (_request.getIsFile()) {
		LOG_INFO("Handling file CGI");

		// Checks if this extensions has to be handled by a CGI
		LOG_INFO("Checks if this extension has to be handled by a CGI");

		if (route.getCgiHandlers().find(_request.getRessourceExtension()) == route.getCgiHandlers().end() ||
			route.getCgiHandlers().at(_request.getRessourceExtension()).empty())
			LOG_DEBUG("  |- No CGI handlers found for extension:  " + _request.getRessourceExtension() + "\n");
		else {
			LOG_DEBUG("  |- Found:            " + _request.getRessourceExtension());
			LOG_DEBUG("  |- Executable path:  " + route.getCgiHandlers().at(_request.getRessourceExtension()) + "\n");
			handleRequestCGIExecution(route);
		}

	} else {
		LOG_INFO("Handling directory CGI");
	}
}
