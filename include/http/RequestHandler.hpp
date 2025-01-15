/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 15:29:41 by flfische          #+#    #+#             */
/*   Updated: 2025/01/14 10:31:14 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Route.hpp"
#define DEFAULT_CGI_TIMEOUT_MS 5000

class ServerConfig;

class RequestHandler {
		HttpRequest _request;
		HttpResponse _response;
		bool _cgiExecuted = false;
		ServerConfig& _serverConfig;
		Route _matchedRoute;

		// General Functions
		void findMatchingRoute();

		// CGI handler
		void handleRequestCGI(Route& route);
		void handleRequestCGIExecution(const Route& route);

		// Request handlers
		// GET request handlers
		HttpResponse handleGetRequest();
		HttpResponse handleGetFile();
		HttpResponse handleGetDirectory();

		// POST request handlers
		[[nodiscard]] HttpResponse handlePostRequest();
		[[nodiscard]] HttpResponse handlePostMultipart();
		[[nodiscard]] HttpResponse handleFileUpload(const std::string& part, const std::string& contentDisposition);

		// DELETE request handler
		[[nodiscard]] HttpResponse handleDeleteRequest();

		// Autoindex handler
		void handleAutoindex(const std::string& path);
		[[nodiscard]] std::string buildDirectoryListingHTML(const std::string& path) const;

		// Redirect Request
		[[nodiscard]] HttpResponse handleRedirectRequest();

	public:
		explicit RequestHandler(ServerConfig& serverConfig);
		~RequestHandler() = default;
		RequestHandler(const RequestHandler& other) = delete;
		RequestHandler& operator=(const RequestHandler& other) = delete;

		[[nodiscard]] ServerConfig& getConfig() const;
		void setConfig(const ServerConfig & server_config) const;
		HttpResponse handleRequest(const HttpRequest& request);
		HttpResponse buildDefaultResponse(Http::Status code, std::optional<HttpRequest> request = std::nullopt);
};
