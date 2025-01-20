/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 15:29:41 by flfische          #+#    #+#             */
/*   Updated: 2025/01/18 13:36:57 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/types.h>

#include <chrono>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Route.hpp"
#include "optional"

enum cgiState { NONE, WRITING, WAITING, READING, FINISHED };

class ServerConfig;

class RequestHandler {
		HttpRequest _request;
		HttpResponse _response = HttpResponse();
		ServerConfig& _serverConfig;
		Route _matchedRoute;

		bool _parsingDone = false;

		long long _bytesReadFromFile = 0;
		long long _bytesWrittenToFile = 0;

		bool _cgi_valid = false;
		pid_t _cgi_pid = 0;
		std::chrono::milliseconds _cgi_startTime = std::chrono::milliseconds(0);
		cgiState _cgi_state = cgiState::NONE;
		int _cgi_pipeIn[2] = {0, 0};
		int _cgi_pipeOut[2] = {0, 0};
		int _cgi_status = 0;

		std::string _fileName = "";

		// General Functions
		void findMatchingRoute();

		// CGI handler
		[[nodiscard]] bool checkRequestCGI(Route& route);
		void handleRequestCGIExecution(const Route& route);

		// Request handlers
		// GET request handlers
		bool handleGetRequest();
		bool handleGetFile();
		bool handleGetDirectory();

		// POST request handlers
		[[nodiscard]] bool handlePostRequest();
		bool setFileNameAndBody(const std::string& part, const std::string& contentDisposition);
		[[nodiscard]] bool handlePostMultipart();
		[[nodiscard]] bool handleFileUpload();

		// DELETE request handler
		void handleDeleteRequest();

		// Autoindex handler
		void handleAutoindex(const std::string& path);
		[[nodiscard]] std::string buildDirectoryListingHTML(const std::string& path) const;

		// Redirect Request
		[[nodiscard]] HttpResponse handleRedirectRequest();

	public:
		~RequestHandler() = default;
		RequestHandler(const RequestHandler& other) = delete;
		RequestHandler& operator=(const RequestHandler& other) = delete;

		RequestHandler(ServerConfig& serverConfig);
		[[nodiscard]] ServerConfig& getConfig() const;
		void setConfig(const ServerConfig& server_config) const;
		bool handleRequest(const HttpRequest& request);
		HttpResponse getResponse();
		HttpResponse buildDefaultResponse(Http::Status code, std::optional<HttpRequest> request = std::nullopt);
};
