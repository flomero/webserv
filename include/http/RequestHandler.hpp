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
#include "optional"

#define DEFAULT_CGI_TIMEOUT_MS 5000

class ServerConfig;

class RequestHandler {
		HttpRequest _request;
		HttpResponse _response = HttpResponse();
		bool _cgiExecuted = false;
		ServerConfig& _serverConfig;
		Route _matchedRoute;

		size_t _bytesReadFromFile = 0;
		bool _parsingDone = false;

		// General Functions
		void findMatchingRoute();

		// CGI handler
		void handleRequestCGI(Route& route);
		void handleRequestCGIExecution(const Route& route);

		// Request handlers
		// GET request handlers
		bool handleGetRequest();
		bool handleGetFile();
		bool handleGetDirectory();

		// POST request handlers
		[[nodiscard]] bool handlePostRequest();
		[[nodiscard]] HttpResponse handlePostMultipart();
		[[nodiscard]] HttpResponse handleFileUpload(const std::string& part, const std::string& contentDisposition);

		// DELETE request handler
		void handleDeleteRequest();

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
		void setConfig(const ServerConfig& server_config) const;
		bool handleRequest(const HttpRequest& request);
		HttpResponse getResponse();
		HttpResponse buildDefaultResponse(Http::Status code, std::optional<HttpRequest> request = std::nullopt);
};
