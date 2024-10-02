/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:02:16 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/02 14:44:31 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/configuration/Parser.hpp"

Parser::Parser(Lexer& lexer): _lexer(lexer), _currentToken(lexer.nextToken()) {}


void Parser::expect(TokenType type) {
	if (_currentToken.type != type)
		throw std::runtime_error("Unexpected token");

	_currentToken = _lexer.nextToken();
}

std::vector<Server> Parser::parse() {
	std::vector<Server> servers;
	while (_currentToken.type != TOKEN_EOF) {
		try {
			auto tmp = parseServer();
			servers.push_back(tmp);
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			exit(1);
		}
	}
	return servers;
}

Server Parser::parseServer() {
	expect(TOKEN_SERVER);
	expect(TOKEN_OPEN_BRACE);

	Server server;

	while (_currentToken.type != TOKEN_CLOSE_BRACE) {
		if (_currentToken.type == TOKEN_PORT) {
			expect(TOKEN_PORT);
			server.port = std::stoi(_currentToken.value);
			expect(TOKEN_NUMBER);
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_HOST) {
			expect(TOKEN_HOST);
			server.host = _currentToken.value;
			expect(TOKEN_IP_V4);
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_SERVER_NAME) {
			expect(TOKEN_SERVER_NAME);
			server.serverName = _currentToken.value;
			expect(TOKEN_STRING);
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_ERROR_PAGE) {
			expect(TOKEN_ERROR_PAGE);
			int code = std::stoi(_currentToken.value);
			expect(TOKEN_NUMBER);
			std::string path = _currentToken.value;
			expect(TOKEN_STRING);
			server.errorPages[code] = path;
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_CLIENT_BODY_SIZE) {
			expect(TOKEN_CLIENT_BODY_SIZE);
			server.clientBodySize = std::stoul(_currentToken.value);
			expect(TOKEN_NUMBER);
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_LOCATION) {
			server.routes.push_back(parseRoute());
		} else {
			throw std::runtime_error("Unexpected token in server body");
		}
	}

	expect(TOKEN_CLOSE_BRACE);
	return server;
}

Route Parser::parseRoute() {
	expect(TOKEN_LOCATION);
	Route route;
	route.path = _currentToken.value;
	expect(TOKEN_STRING);
	expect(TOKEN_OPEN_BRACE);

	while (_currentToken.type != TOKEN_CLOSE_BRACE) {
		if (_currentToken.type == TOKEN_METHODS) {
			expect(TOKEN_METHODS);
			while (_currentToken.type == TOKEN_STRING) {
				route.methods.push_back(_currentToken.value);
				_currentToken = _lexer.nextToken();
			}
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_ROOT) {
			expect(TOKEN_ROOT);
			route.root = _currentToken.value;
			expect(TOKEN_STRING);
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_INDEX) {
			expect(TOKEN_INDEX);
			route.index = _currentToken.value;
			expect(TOKEN_STRING);
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_AUTOINDEX) {
			expect(TOKEN_AUTOINDEX);
			if (_currentToken.type == TOKEN_ON) {
				route.autoindex = true;
			} else if (_currentToken.type == TOKEN_OFF) {
				route.autoindex = false;
			}
			_currentToken = _lexer.nextToken();
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_UPLOAD_DIR) {
			expect(TOKEN_UPLOAD_DIR);
			route.uploadDir = _currentToken.value;
			expect(TOKEN_STRING);
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_CGI) {
			expect(TOKEN_CGI);
			std::string ext = _currentToken.value;
			expect(TOKEN_STRING);
			std::string handler = _currentToken.value;
			expect(TOKEN_STRING);
			route.cgiHandlers[ext] = handler;
			expect(TOKEN_SEMICOLON);
		} else if (_currentToken.type == TOKEN_REDIRECT) {
			expect(TOKEN_REDIRECT);
			route.redirect = _currentToken.value;
			expect(TOKEN_STRING);
			expect(TOKEN_SEMICOLON);
		} else {
			throw std::runtime_error("Unexpected token in route");
		}
	}

	expect(TOKEN_CLOSE_BRACE);
	return route;
}
