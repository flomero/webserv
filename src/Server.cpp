/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:56:20 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/06 14:21:16 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

// Constructor
Server::Server() : _port(80), _requestTimeout(60), _clientMaxBodySize(1048576), _host("127.0.0.1") {}

// Getters
int Server::getPort() const { return _port; }
size_t Server::getRequestTimeout() const { return _requestTimeout; }
size_t Server::getClientMaxBodySize() const { return _clientMaxBodySize; }
const std::string& Server::getHost() const { return _host; }
const std::string& Server::getIndex() const { return _index; }
const std::string& Server::getRoot() const { return _root; }
const std::string& Server::getUploadDir() const { return _uploadDir; }
const std::string& Server::getServerName() const { return _serverName; }
const std::vector<Route>& Server::getRoutes() const { return _routes; }
const std::map<int, std::string>& Server::getErrorPages() const { return _errorPages; }

// Setters
void Server::setPort(int port) { _port = port; }
void Server::setRequestTimeout(size_t timeout) { _requestTimeout = timeout; }
void Server::setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
void Server::setHost(const std::string& host) { _host = host; }
void Server::setIndex(const std::string& index) { _index = index; }
void Server::setRoot(const std::string& root) { _root = root; }
void Server::setUploadDir(const std::string& dir) { _uploadDir = dir; }
void Server::setServerName(const std::string& name) { _serverName = name; }
void Server::setRoutes(const std::vector<Route>& routes) { _routes = routes; }
void Server::setErrorPages(const std::map<int, std::string>& pages) { _errorPages = pages; }

// Overload "<<" operator
std::ostream& operator<<(std::ostream& os, const Server& server) {
	os << std::left << std::setw(32) << server.getServerName() << server.getHost() << ":" << server.getPort() << "\n";

	if (server.getIndex() != "") {os << std::left << std::setw(32) << "  |- index: " << server.getIndex() << "\n";}
	if (server.getRoot() != "") {os << std::left << std::setw(32) << "  |- root: " << server.getRoot() << "\n";}
	if (server.getUploadDir() != "") {os << std::left << std::setw(32) << "  |- upload dir: " << server.getUploadDir() << "\n";}

	os << std::left << std::setw(32) << "  |- client max body size: " << server.getClientMaxBodySize() << " bytes\n";
	os << std::left << std::setw(32) << "  |- request timeout: " << server.getRequestTimeout() << " seconds\n";

	os << "  |- error pages: \n";
	for (const auto& page : server.getErrorPages())
		os << std::left << std::setw(8) << "    |- " << page.first << " -> " << page.second << "\n";

	os << "  |- routes: \n";
	for (const auto& route : server.getRoutes())
		os << "    |- " << route << "\n"; // Use Route's overloaded << operator

	return os;
}
