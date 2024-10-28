/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestError.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 19:57:06 by flfische          #+#    #+#             */
/*   Updated: 2024/10/28 20:47:04 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

HttpResponse RequestHandler::buildErrorResponse(Http::Status code) {
	HttpResponse response(code);

	std::optional<std::string> errorPage = _serverConfig.getErrorPage(code);

	if (errorPage.has_value()) {
		std::ifstream file(errorPage.value());
		if (file.is_open()) {
			// Read the file into a string
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			response.setBody(content);
			return response;
		}
	}

	// TODO: add more sophisticated error pages
	response.setBody(Http::getStatusMessage(code));
	return response;
}
