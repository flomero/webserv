/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:56:20 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/15 11:11:59 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

// Constructor
Server::Server()
	: _port(8080),  // Changed port from 80 to 8080
	  _requestTimeout(60),
	  _clientMaxBodySize(1048576),
	  _host("127.0.0.1"),
	  _serverSocket(_port) {
	_serverSocket.bind();
	_serverSocket.listen();
}


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
const std::map<int, std::string>& Server::getErrorPages() const {
	return _errorPages;
}

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
void Server::setErrorPages(const std::map<int, std::string>& pages) {
	_errorPages = pages;
}

// Overload "<<" operator
std::ostream& operator<<(std::ostream& os, const Server& server) {
	os << std::left << std::setw(32) << COLOR(BLUE, server.getServerName())
	   << BLUE << server.getHost() << ":" << server.getPort() << RESET_COLOR
	   << "\n";

	if (server.getIndex() != "") {
		os << std::left << std::setw(32) << "  |- index: " << server.getIndex()
		   << "\n";
	}
	if (server.getRoot() != "") {
		os << std::left << std::setw(32) << "  |- root: " << server.getRoot()
		   << "\n";
	}
	if (server.getUploadDir() != "") {
		os << std::left << std::setw(32)
		   << "  |- upload dir: " << server.getUploadDir() << "\n";
	}

	os << std::left << std::setw(32)
	   << "  |- client max body size: " << server.getClientMaxBodySize()
	   << " bytes\n";
	os << std::left << std::setw(32)
	   << "  |- request timeout: " << server.getRequestTimeout() << " ms\n";

	if (server.getErrorPages().size() != 0) {
		os << "  |- error pages: \n";
		for (const auto& page : server.getErrorPages())
			os << std::left << std::setw(8) << "    |- " << page.first << " -> "
			   << page.second << "\n";
	}

	os << "  |- routes: \n";
	for (const auto& route : server.getRoutes())
		os << "    |- " << route << "\n";  // Use Route's overloaded << operator

	return os;
}

void Server::run() {
	while (true) {
		acceptNewClient();
		handleClients();
	}
}

void Server::acceptNewClient() {
	try {
		int client_fd = _serverSocket.accept();
		LOG_INFO("New client connected");
		auto* client = new ClientConnection(client_fd);
		_clients.push_back(client);
	} catch (const std::exception& e) {
		// LOG_ERROR(e.what());
	}
}

void Server::handleClients() {
	for (size_t i = 0; i < _clients.size(); ++i) {
		if (_clients[i]->isDisconnected()) {
			delete _clients[i];
			_clients.erase(_clients.begin() + i);
			--i;
		} else {
			_clients[i]->processRequest();
		}
	}
}
