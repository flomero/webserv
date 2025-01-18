/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCGI.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 16:54:21 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/18 13:35:14 by flfische         ###   ########.fr       */
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
 * @param route
 */
bool RequestHandler::checkRequestCGI(Route& route) {
	LOG_INFO("Entered checkRequestCGI");

	if (_request.getIsFile()) {
		LOG_DEBUG("Handling file CGI");

		// Checks if this extensions has to be handled by a CGI
		LOG_DEBUG("Checks if this extension has to be handled by a CGI");

		if (route.getCgiHandlers().find(_request.getResourceExtension()) == route.getCgiHandlers().end() ||
			route.getCgiHandlers().at(_request.getResourceExtension()).empty()) {
			LOG_DEBUG("  |- No CGI handlers found for extension:  " + _request.getResourceExtension() + "\n");
			return false;
		} else {
			LOG_DEBUG("  |- Found:            " + _request.getResourceExtension());
			LOG_DEBUG("  |- Executable path:  " + route.getCgiHandlers().at(_request.getResourceExtension()) + "\n");
			return true;
		}

	} else {
		LOG_INFO("Handling directory CGI");
		return false;
	}
}
