/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:20:10 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/06 16:04:10 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <string>
# include <vector>

# define COMMENT_PLACEHOLDER '#'

enum TokenType {
	TOKEN_HTTP,
	TOKEN_SERVER,
	TOKEN_LOCATION,
	TOKEN_LISTEN,
	TOKEN_SERVER_NAME,
	TOKEN_ROOT,
	TOKEN_INDEX,
	TOKEN_CLIENT_BODY_SIZE,
	TOKEN_UPLOAD_DIR,
	TOKEN_REQUEST_TIMEOUT,
	TOKEN_ERROR_PAGE,
	TOKEN_ALLOW_METHODS,
	TOKEN_AUTOINDEX,
	TOKEN_ALIAS,
	TOKEN_CGI,
	TOKEN_RETURN,
	TOKEN_IP_V4,
	TOKEN_NUMBER,
	TOKEN_ON,
	TOKEN_OFF,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_SEMICOLON,
	TOKEN_COMMENT,
	TOKEN_STRING,
	TOKEN_EOF,
	TOKEN_INVALID
};

struct Token {
	TokenType type;
	std::string value;
	int line;
	int column;
};

class Lexer {
	private:
		std::string	_source;
		size_t		_currentPos;
		int			_line;
		int			_column;

		char	getChar();
		char	peekChar();
		void	skipWhitespace();
		void	skipComment();

		Token	parseNumber();
		Token	parseIp_v4(std::string value, int count);
		Token	parseString(std::string value);
		Token	parseKeywordOrString();

	public:
		Lexer(const std::string& source);
		Token nextToken();
		Token nextTokenWhitespace();
};
