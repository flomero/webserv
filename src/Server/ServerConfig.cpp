/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:56:20 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/28 15:17:56 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

// Constructor
ServerConfig::ServerConfig() : _port(80), _requestTimeout(60), _clientMaxBodySize(1048576), _host("127.0.0.1") {}

// Getters
int ServerConfig::getPort() const { return _port; }
size_t ServerConfig::getRequestTimeout() const { return _requestTimeout; }
size_t ServerConfig::getClientMaxBodySize() const { return _clientMaxBodySize; }
const std::string& ServerConfig::getHost() const { return _host; }
const std::string& ServerConfig::getIndex() const { return _index; }
const std::string& ServerConfig::getRoot() const { return _root; }
const std::string& ServerConfig::getUploadDir() const { return _uploadDir; }
const std::string& ServerConfig::getServerName() const { return _serverName; }
const std::vector<Route>& ServerConfig::getRoutes() const { return _routes; }
const std::map<int, std::string>& ServerConfig::getErrorPages() const { return _errorPages; }

// Setters
void ServerConfig::setPort(int port) { _port = port; }
void ServerConfig::setRequestTimeout(size_t timeout) { _requestTimeout = timeout; }
void ServerConfig::setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
void ServerConfig::setHost(const std::string& host) { _host = host; }
void ServerConfig::setIndex(const std::string& index) { _index = index; }
void ServerConfig::setRoot(const std::string& root) { _root = root; }
void ServerConfig::setUploadDir(const std::string& dir) { _uploadDir = dir; }
void ServerConfig::setServerName(const std::string& name) { _serverName = name; }
void ServerConfig::setRoutes(const std::vector<Route>& routes) { _routes = routes; }
void ServerConfig::setErrorPages(const std::map<int, std::string>& pages) { _errorPages = pages; }

// Overload "<<" operator
std::ostream& operator<<(std::ostream& os, const ServerConfig& server) {
	os << std::left << std::setw(32) << COLOR(BLUE, server.getServerName()) << BLUE << server.getHost() << ":"
	   << server.getPort() << RESET_COLOR << "\n";

	if (server.getIndex() != "") {
		os << std::left << std::setw(32) << "  |- index: " << server.getIndex() << "\n";
	}
	if (server.getRoot() != "") {
		os << std::left << std::setw(32) << "  |- root: " << server.getRoot() << "\n";
	}
	if (server.getUploadDir() != "") {
		os << std::left << std::setw(32) << "  |- upload dir: " << server.getUploadDir() << "\n";
	}

	os << std::left << std::setw(32) << "  |- client max body size: " << server.getClientMaxBodySize() << " bytes\n";
	os << std::left << std::setw(32) << "  |- request timeout: " << server.getRequestTimeout() << " ms\n";

	if (server.getErrorPages().size() != 0) {
		os << "  |- error pages: \n";
		for (const auto& page : server.getErrorPages())
			os << std::left << std::setw(8) << "    |- " << page.first << " -> " << page.second << "\n";
	}

	os << "  |- routes: \n";
	for (const auto& route : server.getRoutes())
		os << "    |- " << route << "\n";  // Use Route's overloaded << operator

	return os;
}
