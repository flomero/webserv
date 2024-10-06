/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:54:56 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/06 14:32:15 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <string>
# include <vector>
# include <map>

# include <iostream>
# include "misc/ft_iomanip.hpp"


class Route {
	private:
		std::string							_path;
		std::string							_alias;
		std::vector<std::string>			_methods;
		std::string							_root;
		std::string							_index;
		bool								_autoindex;
		std::string							_uploadDir;
		std::map<std::string, std::string>	_cgiHandlers;
		int									_code;
		std::string							_redirect;

	public:
		// Constructor
		Route();

		// Getters
		const std::string&			getPath() const;
		const std::string&			getAlias() const;
		const std::vector<std::string>& getMethods() const;
		const std::string&			getRoot() const;
		const std::string&			getIndex() const;
		bool						isAutoindex() const;
		const std::string&			getUploadDir() const;
		const std::map<std::string, std::string>& getCgiHandlers() const;
		int							getCode() const;
		const std::string&			getRedirect() const;

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

		// Overload "<<" operator to print Route details
		friend std::ostream& operator<<(std::ostream& os, const Route& route);
};
