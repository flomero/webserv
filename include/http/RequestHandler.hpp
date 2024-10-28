/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 15:29:41 by flfische          #+#    #+#             */
/*   Updated: 2024/10/28 16:42:40 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"

class RequestHandler {
	private:
		RequestHandler(const RequestHandler& other) = delete;
		RequestHandler& operator=(const RequestHandler& other) = delete;
		HttpRequest _request;
		HttpResponse _response;
		ServerConfig _serverConfig;

		// Main request logic

		// CGI handler
		void handleRequestCGI(Route& route);
		void handleRequestCGIExecution(Route& route);

		// Request handlers
		// POST request handlers
		int handlePostRequest();
		int handlePostMultipart();
		int handleFileUpload(const std::string& part, const std::string& contentDisposition);

		// Autoindex handler
		void handleAutoindex(const std::string& path);

	public:
		explicit RequestHandler(ServerConfig& serverConfig);
		HttpResponse handleRequest(std::string raw_request);
		~RequestHandler() = default;
};
