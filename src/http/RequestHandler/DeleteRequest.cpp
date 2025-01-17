/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 14:48:43 by flfische          #+#    #+#             */
/*   Updated: 2025/01/06 15:53:50 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <filesystem>

#include "Logger.hpp"
#include "RequestHandler.hpp"

void RequestHandler::handleDeleteRequest() {
	LOG_DEBUG("Handling DELETE request");

	const std::string& serverSidePath = _request.getServerSidePath();

	if (!std::filesystem::exists(serverSidePath)) {
		LOG_WARN("File or directory not found: " + serverSidePath);
		_response = buildDefaultResponse(Http::NOT_FOUND);
		return;
	}

	if (std::filesystem::is_directory(serverSidePath)) {
		LOG_WARN("Attempt to delete a directory: " + serverSidePath);
		_response = buildDefaultResponse(Http::FORBIDDEN);
	}

	try {
		if (std::filesystem::remove(serverSidePath)) {
			LOG_INFO("File deleted successfully: " + serverSidePath);
			_response.setStatus(Http::NO_CONTENT);
		} else {
			LOG_WARN("Failed to delete file: " + serverSidePath);
			_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
		}
	} catch (const std::filesystem::filesystem_error& e) {
		LOG_ERROR("Filesystem error: " + std::string(e.what()));
		_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
	}
}
