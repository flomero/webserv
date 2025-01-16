/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:56:20 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/16 10:47:27 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

// Constructor
ServerConfig::ServerConfig() : _port(80), _requestTimeout(60), _clientMaxBodySize(1048576), _host("0.0.0.0") {}

// Simple Getters
int ServerConfig::getPort() const { return _port; }

size_t ServerConfig::getRequestTimeout() const { return _requestTimeout; }

size_t ServerConfig::getClientMaxBodySize() const { return _clientMaxBodySize; }

size_t ServerConfig::getClientBodyBufferSize() const { return _clientBodyBufferSize; }

size_t ServerConfig::getClientHeaderBufferSize() const { return _clientHeaderBufferSize; }

const std::string& ServerConfig::getHostIP() const { return _host; }

const std::string& ServerConfig::getIndex() const { return _index; }

const std::string& ServerConfig::getRoot() const { return _root; }

const std::string& ServerConfig::getUploadDir() const { return _uploadDir; }

std::vector<std::string> ServerConfig::getServerNames() const { return _serverNames; }

const std::vector<Route>& ServerConfig::getRoutes() const { return _routes; }

const std::map<int, std::string>& ServerConfig::getErrorPages() const { return _errorPages; }

// Custom Getters
std::optional<std::string> ServerConfig::getErrorPage(const Http::Status code) const {
	const auto it = _errorPages.find(code);
	if (it == _errorPages.end()) {
		return std::nullopt;
	}
	return it->second;
}

// Setters
void ServerConfig::setPort(const int port) { _port = port; }

void ServerConfig::setRequestTimeout(const size_t timeout) { _requestTimeout = timeout; }

void ServerConfig::setClientMaxBodySize(const size_t size) { _clientMaxBodySize = size; }

void ServerConfig::setClientBodyBufferSize(const size_t size) { _clientBodyBufferSize = size; }

void ServerConfig::setClientHeaderBufferSize(const size_t size) { _clientHeaderBufferSize = size; }

void ServerConfig::setHost(const std::string& host) { _host = host; }

void ServerConfig::setIndex(const std::string& index) { _index = index; }

void ServerConfig::setRoot(const std::string& root) { _root = root; }

void ServerConfig::setUploadDir(const std::string& dir) { _uploadDir = dir; }

void ServerConfig::setRoutes(const std::vector<Route>& routes) { _routes = routes; }

void ServerConfig::setErrorPages(const std::map<int, std::string>& pages) { _errorPages = pages; }

void ServerConfig::addServerName(const std::string& name) {
	if (std::find(_serverNames.begin(), _serverNames.end(), name) == _serverNames.end()) {
		_serverNames.push_back(name);
	}
}

// Overload "<<" operator
std::ostream& operator<<(std::ostream& os, const ServerConfig& server) {
	os << std::left << std::setw(32) << COLOR(BLUE, server.getHostIP()) << BLUE << server.getHostIP() << ":"
	   << server.getPort() << RESET_COLOR << "\n";

	if (!server.getServerNames().empty()) {
		os << "  |- server names: \n";
		for (const auto& name : server.getServerNames())
			os << std::left << std::setw(8) << "    |- " << name << std::endl;
	}

	if (!server.getIndex().empty()) {
		os << std::left << std::setw(32) << "  |- index: " << server.getIndex() << "\n";
	}
	if (!server.getRoot().empty()) {
		os << std::left << std::setw(32) << "  |- root: " << server.getRoot() << "\n";
	}
	if (!server.getUploadDir().empty()) {
		os << std::left << std::setw(32) << "  |- upload dir: " << server.getUploadDir() << "\n";
	}

	os << std::left << std::setw(32) << "  |- client max body size: " << server.getClientMaxBodySize() << " bytes\n";
	os << std::left << std::setw(32) << "  |- client body buffer size: " << server.getClientBodyBufferSize()
	   << " bytes\n";
	os << std::left << std::setw(32) << "  |- client header buffer size: " << server.getClientHeaderBufferSize()
	   << " bytes\n";
	os << std::left << std::setw(32) << "  |- request timeout: " << server.getRequestTimeout() << " ms\n";

	if (!server.getErrorPages().empty()) {
		os << "  |- error pages: \n";
		for (const auto& page : server.getErrorPages())
			os << std::left << std::setw(8) << "    |- " << page.first << " -> " << page.second << "\n";
	}

	os << "  |- routes: \n";
	for (const auto& route : server.getRoutes())
		os << "    |- " << route << "\n";  // Use Route's overloaded << operator

	return os;
}
