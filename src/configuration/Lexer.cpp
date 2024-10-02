/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:33:07 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/02 13:56:30 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/configuration/Lexer.hpp"

Lexer::Lexer(const std::string& source):
	_source(source), _currentPos(0), _line(1), _column(0) {}



char Lexer::peekChar() {
	if (_currentPos >= _source.size()) return '\0';
	return _source[_currentPos];
}

char Lexer::getChar() {
	if (_currentPos >= _source.size()) return '\0';
	char c = _source[_currentPos++];
	_column++;
	if (c == '\n') {
		_line++;
		_column = 0;
	}
	return c;
}

void Lexer::skipWhitespace() {
	while (isspace(peekChar()))
		getChar();
}


Token Lexer::nextToken() {
	skipWhitespace();

	char c = peekChar();
	if (c == '\0') return {TOKEN_EOF, "", _line, _column};

	if (c == '{') {
		getChar();
		return {TOKEN_OPEN_BRACE, "{", _line, _column};
	}

	if (c == '}') {
		getChar();
		return {TOKEN_CLOSE_BRACE, "}", _line, _column};
	}

	if (c == ';') {
		getChar();
		return {TOKEN_SEMICOLON, ";", _line, _column};
	}

	if (isdigit(c)) {
		return parseNumber();
	}

	return parseKeywordOrString();
}


Token Lexer::parseNumber() {
	std::string value;
	while (isdigit(peekChar())) {
		value += getChar();
	}
	return {TOKEN_NUMBER, value, _line, _column};
}

Token Lexer::parseString() {
	std::string value;
	while (isalnum(peekChar()) || peekChar() == '/' || peekChar() == '.' || peekChar() == '-' || peekChar() == '_') {
		value += getChar();
	}
	return {TOKEN_STRING, value, _line, _column};
}

Token Lexer::parseKeywordOrString() {
	std::string value;
	while (isalnum(peekChar()) || peekChar() == '_' || peekChar() == '.' || peekChar() == '/') {
		value += getChar();
	}

	if (value == "server") return {TOKEN_SERVER, value, _line, _column};
	if (value == "location") return {TOKEN_LOCATION, value, _line, _column};
	if (value == "port") return {TOKEN_PORT, value, _line, _column};
	if (value == "host") return {TOKEN_HOST, value, _line, _column};
	if (value == "server_name") return {TOKEN_SERVER_NAME, value, _line, _column};
	if (value == "error_page") return {TOKEN_ERROR_PAGE, value, _line, _column};
	if (value == "client_body_size") return {TOKEN_CLIENT_BODY_SIZE, value, _line, _column};
	if (value == "methods") return {TOKEN_METHODS, value, _line, _column};
	if (value == "root") return {TOKEN_ROOT, value, _line, _column};
	if (value == "index") return {TOKEN_INDEX, value, _line, _column};
	if (value == "autoindex") return {TOKEN_AUTOINDEX, value, _line, _column};
	if (value == "upload_dir") return {TOKEN_UPLOAD_DIR, value, _line, _column};
	if (value == "cgi") return {TOKEN_CGI, value, _line, _column};
	if (value == "redirect") return {TOKEN_REDIRECT, value, _line, _column};
	if (value == "on") return {TOKEN_ON, value, _line, _column};
	if (value == "off") return {TOKEN_OFF, value, _line, _column};

	return {TOKEN_STRING, value, _line, _column}; // Default to string if not a keyword
}
