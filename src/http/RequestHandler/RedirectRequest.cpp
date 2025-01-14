/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectRequest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 16:35:58 by flfische          #+#    #+#             */
/*   Updated: 2025/01/06 16:37:46 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "RequestHandler.hpp"

HttpResponse RequestHandler::handleRedirectRequest() {
	int returnCode = _matchedRoute.getCode();
	const std::string& redirectUrl = _matchedRoute.getRedirect();

	if (!redirectUrl.empty()) {
		LOG_INFO("Route has a return directive with redirection.");
		_response.setStatus(static_cast<Http::Status>(returnCode));
		_response.addHeader("Location", redirectUrl);
		return _response;
	} else {
		LOG_INFO("Route has a return directive without redirection.");
		return buildDefaultResponse(static_cast<Http::Status>(returnCode));
	}
}
