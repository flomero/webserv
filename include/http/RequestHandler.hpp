/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 15:29:41 by flfische          #+#    #+#             */
/*   Updated: 2024/12/10 16:20:46 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Route.hpp"

class ServerConfig;

class RequestHandler {
	private:
		HttpRequest _request;
		HttpResponse _response;
		bool _cgiExecuted = false;
		ServerConfig& _serverConfig;
		Route _matchedRoute;

		// General Functions
		void findMatchingRoute();

		// GET request handlers
		HttpResponse handleGetRequest();
		HttpResponse handleGetFile();
		HttpResponse handleGetDirectory();

		// CGI handler
		void handleRequestCGI(Route& route);
		void handleRequestCGIExecution(const Route& route);

		// Request handlers
		// POST request handlers
		[[nodiscard]] HttpResponse handlePostRequest();
		[[nodiscard]] HttpResponse handlePostMultipart();
		[[nodiscard]] HttpResponse handleFileUpload(const std::string& part, const std::string& contentDisposition);

		// Autoindex handler
		void handleAutoindex(const std::string& path);
		[[nodiscard]] std::string buildDirectoryListingHTML(const std::string& path) const;

	public:
		[[nodiscard]] ServerConfig& getConfig() const;
		explicit RequestHandler(ServerConfig& serverConfig);
		~RequestHandler() = default;
		RequestHandler(const RequestHandler& other) = delete;
		RequestHandler& operator=(const RequestHandler& other) = delete;

		HttpResponse handleRequest(const HttpRequest& request);
		HttpResponse buildDefaultResponse(Http::Status code, std::optional<HttpRequest> request = std::nullopt);
};
