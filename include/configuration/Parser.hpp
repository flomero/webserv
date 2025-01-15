/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:58:34 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/28 15:20:25 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <regex>
#include <vector>

#include "Lexer.hpp"
#include "ParsingErrors.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"

#define REGEX_IP_V4                                                            \
	"(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[" \
	"0-9]?[0-9])"
#define REGEX_PORT "[0-9]{1,5}"

class Parser {
	private:
		Lexer& _lexer;
		Token _currentToken;
		std::vector<std::string> _parsingErrors;

		void expect(eTokenType type);
		static std::vector<std::vector<ServerConfig>> splitServerConfigs(const std::vector<ServerConfig>& serverConfigs);
		ServerConfig parseServer();
		Route parseRoute();

	public:
		Parser(Lexer& lexer);
		std::vector<std::vector<ServerConfig>> parse();

		void reportError(eParsingErrors error, std::string expected, std::string found);
		void flushErrors() const;
};
