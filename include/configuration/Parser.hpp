/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:58:34 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/05 14:12:11 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Lexer.hpp"
# include <vector>
# include <map>
# include <exception>
# include <iostream>

struct Route {
	std::string							path;
	std::string							alias;
	std::vector<std::string>			methods;
	std::string							root;
	std::string							index;
	bool								autoindex;
	std::string							uploadDir;
	std::map<std::string, std::string>	cgiHandlers;
	int									code;
	std::string							redirect;
};

struct Server {
	int									port;
	std::string							host;
	std::string							index;
	std::string							root;
	std::string							uploadDir;
	size_t								requestTimeout;
	std::string							serverName;
	std::map<int, std::string>			errorPages;
	size_t								clientBodySize;
	std::vector<Route>					routes;
};

class Parser {
	private:
		Lexer&	_lexer;
		Token	_currentToken;

		void	expect(TokenType type);
		Server	parseServer();
		Route	parseRoute();

	public:
		Parser(Lexer& lexer);
		std::vector<Server> parse();
};
