/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:20:10 by lgreau            #+#    #+#             */
/*   Updated: 2024/11/22 09:36:49 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>

#define COMMENT_PLACEHOLDER '#'

enum eTokenType {
	TOKEN_HTTP,
	TOKEN_SERVER,
	TOKEN_LOCATION,
	TOKEN_LISTEN,
	TOKEN_SERVER_NAME,
	TOKEN_ROOT,
	TOKEN_INDEX,
	TOKEN_CLIENT_MAX_BODY_SIZE,
	TOKEN_CLIENT_BODY_BUFFER_SIZE,
	TOKEN_CLIENT_HEADER_BUFFER_SIZE,
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

const std::map<eTokenType, std::string> tokenToString = {{TOKEN_HTTP, "http"},
														 {TOKEN_SERVER, "server"},
														 {TOKEN_LOCATION, "location"},
														 {TOKEN_LISTEN, "listen"},
														 {TOKEN_SERVER_NAME, "server_name"},
														 {TOKEN_ROOT, "root"},
														 {TOKEN_INDEX, "index"},
														 {TOKEN_CLIENT_MAX_BODY_SIZE, "client_max_body_size"},
														 {TOKEN_CLIENT_BODY_BUFFER_SIZE, "client_body_buffer_size"},
														 {TOKEN_CLIENT_HEADER_BUFFER_SIZE, "client_header_buffer_size"},
														 {TOKEN_UPLOAD_DIR, "upload_dir"},
														 {TOKEN_REQUEST_TIMEOUT, "request_timeout"},
														 {TOKEN_ERROR_PAGE, "error_page"},
														 {TOKEN_ALLOW_METHODS, "allow_methods"},
														 {TOKEN_ALIAS, "alias"},
														 {TOKEN_AUTOINDEX, "autoindex"},
														 {TOKEN_CGI, "cgi"},
														 {TOKEN_RETURN, "return"},

														 {TOKEN_IP_V4, "ip_v4"},
														 {TOKEN_NUMBER, "number"},
														 {TOKEN_ON, "on"},
														 {TOKEN_OFF, "off"},

														 {TOKEN_OPEN_BRACE, "{"},
														 {TOKEN_CLOSE_BRACE, "}"},
														 {TOKEN_SEMICOLON, ";"},

														 {TOKEN_EOF, "EOF"}};

struct Token {
		eTokenType type;
		std::string value;
		int line;
		int column;
};

class Lexer {
	private:
		std::string _source_name;
		std::string _source_content;
		size_t _currentPos;
		int _line;
		int _column;

		char getChar();
		char peekChar() const;
		void skipWhitespace();
		void skipComment();

		Token parseNumber();
		Token parseIp_v4(std::string value, int count);
		Token parseString(std::string value);
		Token parseKeywordOrString();

	public:
		Lexer(const std::string& source_name, const std::string& source_content);
		Token nextToken();
		Token nextTokenWhitespace();

		// void reportError(const std::string &message, const std::string
		// &expected, const std::string &found) const;
		std::string getErrorPrefix() const;
};
