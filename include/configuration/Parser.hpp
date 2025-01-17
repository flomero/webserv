/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:58:34 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/17 14:36:03 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#include "Lexer.hpp"
#include "ParsingErrors.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"
#include "globals.hpp"

#define REGEX_PORT "[0-9]{1,5}"

class Parser {
	private:
		Lexer& _lexer;
		Token _currentToken;
		std::vector<std::string> _parsingErrors;

		void expect(eTokenType type);
		static std::vector<std::vector<ServerConfig>> splitServerConfigs(
			const std::vector<ServerConfig>& serverConfigs);
		ServerConfig parseServer();
		Route parseRoute();

	public:
		Parser(Lexer& lexer);
		std::vector<std::vector<ServerConfig>> parse();

		void reportError(eParsingErrors error, std::string expected, std::string found);
		void flushErrors() const;
};
