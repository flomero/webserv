/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 15:29:41 by flfische          #+#    #+#             */
/*   Updated: 2024/10/31 16:49:17 by flfische         ###   ########.fr       */
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
		ServerConfig& _serverConfig;
		Route _matchedRoute;

		RequestHandler(const RequestHandler& other) = delete;
		RequestHandler& operator=(const RequestHandler& other) = delete;

		// General Functions
		void findMatchingRoute();

		// GET request handlers
		HttpResponse handleGetRequest();
		HttpResponse handleGetFile();
		HttpResponse handleGetDirectory();

		// CGI handler
		void handleRequestCGI(Route& route);
		void handleRequestCGIExecution(Route& route);

		// Request handlers
		// POST request handlers
		[[nodiscard]] HttpResponse handlePostRequest();
		[[nodiscard]] HttpResponse handlePostMultipart();
		[[nodiscard]] HttpResponse handleFileUpload(const std::string& part, const std::string& contentDisposition);

		// Autoindex handler
		void handleAutoindex(const std::string& path);

	public:
		explicit RequestHandler(ServerConfig& serverConfig);
		~RequestHandler() = default;

		HttpResponse handleRequest(HttpRequest& request);
		HttpResponse buildDefaultResponse(Http::Status code);
};
