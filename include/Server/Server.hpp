/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:45:24 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/15 11:14:38 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpStatus.hpp"
#include "Logger.hpp"
#include "Route.hpp"
#include "misc/ft_iomanip.hpp"

class Server {
	private:
		int _port;

		size_t _requestTimeout;
		size_t _clientMaxBodySize;

		std::string _host;
		std::string _index;
		std::string _root;
		std::string _uploadDir;
		std::string _serverName;

		std::vector<Route> _routes;
		std::map<int, std::string> _errorPages;

	public:
		// Constructor
		Server();

		// Getters
		[[nodiscard]] int getPort() const;
		[[nodiscard]] size_t getRequestTimeout() const;
		[[nodiscard]] size_t getClientMaxBodySize() const;
		[[nodiscard]] const std::string& getHost() const;
		[[nodiscard]] const std::string& getIndex() const;
		[[nodiscard]] const std::string& getRoot() const;
		[[nodiscard]] const std::string& getUploadDir() const;
		[[nodiscard]] const std::string& getServerName() const;
		[[nodiscard]] const std::vector<Route>& getRoutes() const;
		[[nodiscard]] const std::map<int, std::string>& getErrorPages() const;

		// Setters
		void setPort(int port);
		void setRequestTimeout(size_t timeout);
		void setClientMaxBodySize(size_t size);
		void setHost(const std::string& host);
		void setIndex(const std::string& index);
		void setRoot(const std::string& root);
		void setUploadDir(const std::string& dir);
		void setServerName(const std::string& name);
		void setRoutes(const std::vector<Route>& routes);
		void setErrorPages(const std::map<int, std::string>& pages);

		// Overload "<<" operator to print Server details
		friend std::ostream& operator<<(std::ostream& os, const Server& server);

		// Request handlers
		// POST request handlers
		int handlePostRequest(HttpRequest& request);
		int handlePostMultipart(HttpRequest& request);
		int handleFileUpload(const std::string& part,
							 const std::string& contentDisposition);
};
