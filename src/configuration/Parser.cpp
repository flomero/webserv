/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:02:16 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/06 13:41:45 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/configuration/Parser.hpp"

Parser::Parser(Lexer& lexer): _lexer(lexer), _currentToken(lexer.nextToken()) {}


void Parser::expect(TokenType type) {
	// std::cout << "[Debug] - currentToken: " << _currentToken.value << std::endl;
	// std::cout << "        - currentToken: " << _currentToken.type << std::endl;
	// std::cout << "        - expected    : " << type << std::endl << std::endl;

	if (_currentToken.type != type)
		throw std::runtime_error("Unexpected token");

	_currentToken = _lexer.nextToken();
}

std::vector<Server> Parser::parse() {
	std::vector<Server> servers;
	expect(TOKEN_HTTP);
	expect(TOKEN_OPEN_BRACE);

	while (_currentToken.type != TOKEN_CLOSE_BRACE) {
		servers.push_back(parseServer());
	}

	expect(TOKEN_CLOSE_BRACE);
	return servers;
}

Server Parser::parseServer() {
	expect(TOKEN_SERVER);
	expect(TOKEN_OPEN_BRACE);

	Server server;

	while (_currentToken.type != TOKEN_CLOSE_BRACE) {
		switch (_currentToken.type) {
			case TOKEN_LISTEN: {
				expect(TOKEN_LISTEN);
				if (_currentToken.type == TOKEN_IP_V4) {
					server.host = _currentToken.value;
					_currentToken = _lexer.nextToken(); // Consume IP

					if (_currentToken.type == TOKEN_STRING
					 && _currentToken.value[0] == ':') { // In the form ":8080"
						server.port = std::stoi(_currentToken.value.substr(1, _currentToken.value.size() - 1));
						_currentToken = _lexer.nextToken();
					} else {
						server.port = 80; // Default port
					}
				} else if (_currentToken.type == TOKEN_NUMBER) {
					server.port = std::stoi(_currentToken.value);
					server.host = std::string("0.0.0.0"); // Default IP
					_currentToken = _lexer.nextToken(); // Consume port
				} else {
					throw std::runtime_error("Invalid listen value");
				}
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_SERVER_NAME:
				expect(TOKEN_SERVER_NAME);
				server.serverName = _currentToken.value;
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_ROOT:
				expect(TOKEN_ROOT);
				server.root = _currentToken.value;
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_INDEX:
				expect(TOKEN_INDEX);
				server.index = _currentToken.value;
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_CLIENT_BODY_SIZE: {
				expect(TOKEN_CLIENT_BODY_SIZE);
				server.clientBodySize = 0;
				size_t mbValue = std::stoul(_currentToken.value);
				expect(TOKEN_NUMBER);
				while (_currentToken.type == TOKEN_STRING) {
					switch(_currentToken.value[0]) {
						case 'k': case 'K':
							mbValue *= 1024;
							break;
						case 'm': case 'M':
							mbValue *= 1024 * 1024;
							break;
						case 'g': case 'G':
							mbValue *= 1024 * 1024 * 1024;
							break;
					}
					_currentToken = _lexer.nextToken(); // Moves past the suffix

					if (_currentToken.type != TOKEN_NUMBER) break;
					server.clientBodySize += mbValue;
					mbValue = std::stoul(_currentToken.value);

					_currentToken = _lexer.nextToken(); // Move past the number
				}
				server.clientBodySize += mbValue;
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_UPLOAD_DIR:
				expect(TOKEN_UPLOAD_DIR);
				server.uploadDir = _currentToken.value;
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_REQUEST_TIMEOUT: {
				expect(TOKEN_REQUEST_TIMEOUT);
				server.requestTimeout = 0;
				size_t msValue = 1000 * std::stoul(_currentToken.value); // By default read in seconds
				expect(TOKEN_NUMBER);
				while (_currentToken.type == TOKEN_STRING) {
					if (_currentToken.value == "ms") // 1/1000 th of a second
						msValue /= 1000;
					else if (_currentToken.value == "m") // 60 seconds
						msValue *= 60;
					else if (_currentToken.value == "h") // 60 minutes
						msValue *= 60 * 60;
					else if (_currentToken.value == "d") // 24 hours
						msValue *= 24 * 60 * 60;
					else if (_currentToken.value == "w") // 7 days
						msValue *= 7 * 24 * 60 * 60;
					else if (_currentToken.value == "M") // 30 days
						msValue *= 30 * 24 * 60 * 60;
					else if (_currentToken.value == "y") // 365 days
						msValue *= 365 * 24 * 60 * 60;


					_currentToken = _lexer.nextToken(); // Moves past the suffix

					if (_currentToken.type != TOKEN_NUMBER) break;
					server.requestTimeout += msValue;
					msValue = 1000 * std::stoul(_currentToken.value);

					_currentToken = _lexer.nextToken(); // Move past the number
				}
				server.requestTimeout += msValue;
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_ERROR_PAGE: {
				expect(TOKEN_ERROR_PAGE);
				int code = std::stoi(_currentToken.value);
				expect(TOKEN_NUMBER);
				std::string path = _currentToken.value;
				expect(TOKEN_STRING);
				server.errorPages[code] = path;
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_LOCATION:
				server.routes.push_back(parseRoute());
				break;

			default:
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
		switch (_currentToken.type) {
			case TOKEN_ALLOW_METHODS:
				expect(TOKEN_ALLOW_METHODS);
				while (_currentToken.type == TOKEN_STRING) {
					route.methods.push_back(_currentToken.value);
					_currentToken = _lexer.nextToken();
				}
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_ALIAS:
				expect(TOKEN_ALIAS);
				route.alias = _currentToken.value;
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_AUTOINDEX:
				expect(TOKEN_AUTOINDEX);
				if (_currentToken.type == TOKEN_ON) {
					route.autoindex = true;
				} else if (_currentToken.type == TOKEN_OFF) {
					route.autoindex = false;
				}
				_currentToken = _lexer.nextToken();
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_CGI: {
				expect(TOKEN_CGI);
				std::string ext = _currentToken.value;
				expect(TOKEN_STRING);
				std::string handler = _currentToken.value;
				expect(TOKEN_STRING);
				route.cgiHandlers[ext] = handler;
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_RETURN: {
				expect(TOKEN_RETURN);
				route.code = std::stoi(_currentToken.value);
				route.redirect = std::string("");
				expect(TOKEN_NUMBER);
				while (_currentToken.type == TOKEN_STRING) {
					route.redirect.append(_currentToken.value);
					_currentToken = _lexer.nextToken();
				}
				expect(TOKEN_SEMICOLON);
				break;
			}

			default:
				throw std::runtime_error("Unexpected token in route body");
		}
	}

	expect(TOKEN_CLOSE_BRACE);
	return route;
}