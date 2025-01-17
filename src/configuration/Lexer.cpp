/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:33:07 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/17 14:58:21 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"

#include <sstream>

Lexer::Lexer(const std::string& source_name, const std::string& source_content)
	: _source_name(source_name), _source_content(source_content), _currentPos(0), _line(1), _column(0) {}

char Lexer::peekChar() const {
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
	while (currentChar != '\0' && currentChar != ';' && currentChar != '{' && currentChar != '}') {
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

Token Lexer::parseIp_v4(std::string value, const int count) {
	if (peekChar() != '.') {
		if (count != 4)
			return parseString(value);	// Defaults to string
		return {TOKEN_IP_V4, value, _line, _column};
	}

	value += getChar();
	while (isdigit(peekChar()))	 // Flexible on empty number = 0
		value += getChar();

	return parseIp_v4(value, count + 1);
}

Token Lexer::parseString(std::string value) {
	while (isalnum(peekChar()) || (peekChar() >= '<' && peekChar() <= '@') ||
		   (peekChar() >= '-' && peekChar() <= '/') || peekChar() == '!' || (peekChar() >= '$' && peekChar() <= '&') ||
		   (peekChar() >= '*' && peekChar() <= '+') || peekChar() == '_' || peekChar() == ':' || peekChar() == '.')
		value += getChar();

	return {TOKEN_STRING, value, _line, _column};
}

Token Lexer::parseKeywordOrString() {
	std::string value;
	while (isalnum(peekChar()) || (peekChar() >= '<' && peekChar() <= '@') ||
		   (peekChar() >= '-' && peekChar() <= '/') || peekChar() == '!' || (peekChar() >= '$' && peekChar() <= '&') ||
		   (peekChar() >= '*' && peekChar() <= '+') || peekChar() == '_' || peekChar() == ':')
		value += getChar();

	for (const auto& [fst, snd] : tokenToString) {
		if (value == snd)
			return {fst, value, _line, _column};
	}

	return {TOKEN_STRING, value, _line, _column};  // Default to string if not a keyword
}

std::string Lexer::getErrorPrefix() const {
	std::ostringstream errorMsg;

	errorMsg << _source_name << ":" << _line << ":" << _column << ": ";

	return errorMsg.str();
}
