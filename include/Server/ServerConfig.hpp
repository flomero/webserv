/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                         :+:      :+: :+:
 */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:45:24 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/22 15:33:51 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpStatus.hpp"
#include "Route.hpp"

class ServerConfig {
		int _port;

		size_t _requestTimeout;

		size_t _clientMaxBodySize = 0;
		size_t _clientBodyBufferSize = 8192;
		size_t _clientHeaderBufferSize = 1024;

		std::string _host;
		std::string _index;
		std::string _root;
		std::string _uploadDir;
		std::vector<std::string> _serverNames = {};

		std::vector<Route> _routes;
		std::map<int, std::string> _errorPages;

	public:
		// Constructor
		ServerConfig();

		// Getters
		[[nodiscard]] int getPort() const;
		[[nodiscard]] size_t getRequestTimeout() const;
		[[nodiscard]] size_t getClientMaxBodySize() const;
		[[nodiscard]] size_t getClientBodyBufferSize() const;
		[[nodiscard]] size_t getClientHeaderBufferSize() const;
		[[nodiscard]] const std::string& getHostIP() const;
		[[nodiscard]] const std::string& getIndex() const;
		[[nodiscard]] const std::string& getRoot() const;
		[[nodiscard]] const std::string& getUploadDir() const;
		[[nodiscard]] std::vector<std::string> getServerNames() const;
		[[nodiscard]] const std::vector<Route>& getRoutes() const;
		[[nodiscard]] const std::map<int, std::string>& getErrorPages() const;
		[[nodiscard]] std::optional<std::string> getErrorPage(Http::Status code) const;

		// Setters
		void setPort(int port);
		void setRequestTimeout(size_t timeout);
		void setClientMaxBodySize(size_t size);
		void setClientBodyBufferSize(size_t size);
		void setClientHeaderBufferSize(size_t size);
		void setHost(const std::string& host);
		void setIndex(const std::string& index);
		void setRoot(const std::string& root);
		void setUploadDir(const std::string& dir);
		void setRoutes(const std::vector<Route>& routes);
		void setErrorPages(const std::map<int, std::string>& pages);

		void addServerName(const std::string& name);

		// Overload "<<" operator to print ServerConfig details
		friend std::ostream& operator<<(std::ostream& os, const ServerConfig& server);
};
