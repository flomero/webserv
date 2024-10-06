/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:45:24 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/06 13:54:37 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Route.hpp"

# include <string>
# include <vector>
# include <map>
# include <exception>

# include <iostream>
# include <iomanip>

class Server {
	private:
		int							_port;

		size_t						_requestTimeout;
		size_t						_clientMaxBodySize;

		std::string					_host;
		std::string					_index;
		std::string					_root;
		std::string					_uploadDir;
		std::string					_serverName;

		std::vector<Route>			_routes;
		std::map<int, std::string>	_errorPages;

	public:
		// Constructor
		Server();

		// Getters
		int									getPort() const;
		size_t								getRequestTimeout() const;
		size_t								getClientMaxBodySize() const;
		const std::string&					getHost() const;
		const std::string&					getIndex() const;
		const std::string&					getRoot() const;
		const std::string&					getUploadDir() const;
		const std::string&					getServerName() const;
		const std::vector<Route>&			getRoutes() const;
		const std::map<int, std::string>&	getErrorPages() const;

		// Setters
		void	setPort(int port);
		void	setRequestTimeout(size_t timeout);
		void	setClientMaxBodySize(size_t size);
		void	setHost(const std::string& host);
		void	setIndex(const std::string& index);
		void	setRoot(const std::string& root);
		void	setUploadDir(const std::string& dir);
		void	setServerName(const std::string& name);
		void	setRoutes(const std::vector<Route>& routes);
		void	setErrorPages(const std::map<int, std::string>& pages);

		// Overload "<<" operator to print Server details
		friend std::ostream& operator<<(std::ostream& os, const Server& server);

};
