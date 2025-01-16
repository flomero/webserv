/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:54:56 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/14 15:28:44 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "misc/ft_iomanip.hpp"

class Route {
	private:
		std::string _path;
		std::string _alias;
		std::vector<std::string> _methods;
		std::string _root;
		std::string _index;
		bool _autoindex;
		std::string _uploadDir;
		std::map<std::string, std::string> _cgiHandlers;
		int _code;
		std::string _redirect;
		size_t _clientMaxBodySize = 0;
		size_t _clientBodyBufferSize = 8192;
		size_t _clientHeaderBufferSize = 1024;

	public:
		// Constructor
		Route();

		// Getters
		[[nodiscard]] const std::string& getPath() const;
		[[nodiscard]] const std::string& getAlias() const;
		[[nodiscard]] const std::vector<std::string>& getMethods() const;
		[[nodiscard]] const std::string& getRoot() const;
		[[nodiscard]] const std::string& getIndex() const;
		[[nodiscard]] bool isAutoindex() const;
		[[nodiscard]] const std::string& getUploadDir() const;
		[[nodiscard]] const std::map<std::string, std::string>& getCgiHandlers() const;
		[[nodiscard]] int getCode() const;
		[[nodiscard]] const std::string& getRedirect() const;
		[[nodiscard]] size_t getClientMaxBodySize() const;
		[[nodiscard]] size_t getClientBodyBufferSize() const;
		[[nodiscard]] size_t getClientHeaderBufferSize() const;

		// Setters
		void setPath(const std::string& path);
		void setAlias(const std::string& alias);
		void setMethods(const std::vector<std::string>& methods);
		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
		void setAutoindex(bool autoindex);
		void setUploadDir(const std::string& dir);
		void setCgiHandlers(const std::map<std::string, std::string>& handlers);
		void setCode(int code);
		void setRedirect(const std::string& redirect);
		void setClientMaxBodySize(size_t size);
		void setClientBodyBufferSize(size_t size);
		void setClientHeaderBufferSize(size_t size);

		// Overload "<<" operator to print Route details
		friend std::ostream& operator<<(std::ostream& os, const Route& route);
};
