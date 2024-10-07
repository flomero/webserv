/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:58:34 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/07 17:59:14 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "Lexer.hpp"
# include "../Server.hpp"
# include "../Route.hpp"
# include "ParsingErrors.hpp"

# include <vector>
# include <map>
# include <exception>
# include <iostream>

class Parser {
	private:
		Lexer&						_lexer;
		Token						_currentToken;
		std::vector<std::string>	_parsingErrors;

		void	expect(eTokenType type);
		Server	parseServer();
		Route	parseRoute();

	public:
		Parser(Lexer& lexer);
		std::vector<Server> parse();

		void reportError(eParsingErrors error, std::string expected, std::string found);
		void flushErrors() const;
};
