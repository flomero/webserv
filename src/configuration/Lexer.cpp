/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:33:07 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/09 14:34:46 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/configuration/Lexer.hpp"

Lexer::Lexer(const std::string& source_name, const std::string& source_content)
	: _source_name(source_name),
	  _source_content(source_content),
	  _currentPos(0),
	  _line(1),
	  _column(0) {}

char Lexer::peekChar() {
	if (_currentPos >= _source_content.size())
		return '\0';
	return _source_content[_currentPos];
}

char Lexer::getChar() {
	if (_currentPos >= _source_content.size())
		return '\0';
	char c = _source_content[_currentPos++];
	_column++;
	if (c == '\n') {
		_line++;
		_column = 0;
	}
	return c;
}

void Lexer::skipWhitespace() {
	while (isspace(peekChar())) getChar();
}

void Lexer::skipComment() {
	while (peekChar() != '\n' && peekChar() != '\0') {
		getChar();
	}
}

Token Lexer::nextToken() {
	skipWhitespace();

	char c = peekChar();
	if (c == '\0')
		return {TOKEN_EOF, "", _line, _column};

	if (c == COMMENT_PLACEHOLDER) {
		skipComment();
		return nextToken();
	}

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

	if (isdigit(c))
		return parseNumber();

	return parseKeywordOrString();
}

Token Lexer::nextTokenWhitespace() {
	std::string value;
	char currentChar = peekChar();

	// Continue looping until we find one of the delimiters
	while (currentChar != '\0' && currentChar != ';' && currentChar != '{' &&
		   currentChar != '}') {
		value += getChar();	 // Consume the current character
		currentChar = peekChar();
	}

	// If we have accumulated anything, return it as a TOKEN_STRING
	if (!value.empty())
		return {TOKEN_STRING, value, _line, _column};

	if (currentChar == '\0') {
		return {TOKEN_EOF, "", _line, _column};
	}
	if (currentChar == '{') {
		return {TOKEN_OPEN_BRACE, "{", _line, _column};
	}
	if (currentChar == '}') {
		return {TOKEN_CLOSE_BRACE, "}", _line, _column};
	}
	if (currentChar == ';') {
		return {TOKEN_SEMICOLON, ";", _line, _column};
	}

	// End of file, return an empty token
	return {TOKEN_EOF, "", _line, _column};
}

Token Lexer::parseNumber() {
	std::string value;
	while (isdigit(peekChar())) value += getChar();

	if (peekChar() == '.')
		return parseIp_v4(value, 1);
	return {TOKEN_NUMBER, value, _line, _column};
}

Token Lexer::parseIp_v4(std::string value, int count) {
	if (peekChar() != '.') {
		if (count != 4)
			return parseString(value);	// Defaults to string
		else
			return {TOKEN_IP_V4, value, _line, _column};
	}

	value += getChar();
	while (isdigit(peekChar()))	 // Flexible on empty number = 0
		value += getChar();

	return parseIp_v4(value, count + 1);
}

Token Lexer::parseString(std::string value) {
	while (isalnum(peekChar()) || (peekChar() >= '<' && peekChar() <= '@') ||
		   (peekChar() >= '-' && peekChar() <= '/') || (peekChar() == '!') ||
		   (peekChar() >= '$' && peekChar() <= '&') ||
		   (peekChar() >= '*' && peekChar() <= '+') || (peekChar() == '_') ||
		   (peekChar() == ':'))
		value += getChar();

	return {TOKEN_STRING, value, _line, _column};
}

Token Lexer::parseKeywordOrString() {
	std::string value;
	while (isalnum(peekChar()) || (peekChar() >= '<' && peekChar() <= '@') ||
		   (peekChar() >= '-' && peekChar() <= '/') || (peekChar() == '!') ||
		   (peekChar() >= '$' && peekChar() <= '&') ||
		   (peekChar() >= '*' && peekChar() <= '+') || (peekChar() == '_') ||
		   (peekChar() == ':'))
		value += getChar();

	if (value == "http")
		return {TOKEN_HTTP, value, _line, _column};
	if (value == "server")
		return {TOKEN_SERVER, value, _line, _column};
	if (value == "location")
		return {TOKEN_LOCATION, value, _line, _column};
	if (value == "listen")
		return {TOKEN_LISTEN, value, _line, _column};
	if (value == "server_name")
		return {TOKEN_SERVER_NAME, value, _line, _column};
	if (value == "root")
		return {TOKEN_ROOT, value, _line, _column};
	if (value == "index")
		return {TOKEN_INDEX, value, _line, _column};
	if (value == "client_max_body_size")
		return {TOKEN_CLIENT_BODY_SIZE, value, _line, _column};
	if (value == "upload_dir")
		return {TOKEN_UPLOAD_DIR, value, _line, _column};
	if (value == "request_timeout")
		return {TOKEN_REQUEST_TIMEOUT, value, _line, _column};
	if (value == "error_page")
		return {TOKEN_ERROR_PAGE, value, _line, _column};
	if (value == "allow_methods")
		return {TOKEN_ALLOW_METHODS, value, _line, _column};
	if (value == "autoindex")
		return {TOKEN_AUTOINDEX, value, _line, _column};
	if (value == "alias")
		return {TOKEN_ALIAS, value, _line, _column};
	if (value == "cgi")
		return {TOKEN_CGI, value, _line, _column};
	if (value == "return")
		return {TOKEN_RETURN, value, _line, _column};
	if (value == "on")
		return {TOKEN_ON, value, _line, _column};
	if (value == "off")
		return {TOKEN_OFF, value, _line, _column};

	return {TOKEN_STRING, value, _line,
			_column};  // Default to string if not a keyword
}

std::string Lexer::getErrorPrefix() const {
	std::ostringstream errorMsg;

	errorMsg << _source_name << ":" << _line << ":" << _column << ": ";

	return errorMsg.str();
}
