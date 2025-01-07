/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:02:16 by lgreau            #+#    #+#             */
/*   Updated: 2024/11/22 09:54:45 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

#include <sstream>

Parser::Parser(Lexer& lexer) : _lexer(lexer), _currentToken(lexer.nextToken()) {}

void Parser::expect(eTokenType type) {
	if (_currentToken.type != type)
		reportError(UNEXPECTED_TOKEN, tokenToString.at(type), tokenToString.at(_currentToken.type));

	_currentToken = _lexer.nextToken();
}

std::vector<ServerConfig> Parser::parse() {
	std::vector<ServerConfig> servers;
	expect(TOKEN_HTTP);
	expect(TOKEN_OPEN_BRACE);

	while (_currentToken.type != TOKEN_CLOSE_BRACE) servers.push_back(parseServer());

	expect(TOKEN_CLOSE_BRACE);

	if (!_parsingErrors.empty())
		throw std::runtime_error("Found some parsing errors");

	return servers;
}

ServerConfig Parser::parseServer() {
	expect(TOKEN_SERVER);
	expect(TOKEN_OPEN_BRACE);

	ServerConfig server;

	while (_currentToken.type != TOKEN_CLOSE_BRACE) {
		switch (_currentToken.type) {
			case TOKEN_LISTEN: {
				expect(TOKEN_LISTEN);
				if (_currentToken.type == TOKEN_IP_V4) {
					server.setHost(_currentToken.value);
					_currentToken = _lexer.nextToken();	 // Consume IP

					if (_currentToken.type == TOKEN_STRING && _currentToken.value[0] == ':') {	// In the form ":8080"
						server.setPort(std::stoi(_currentToken.value.substr(1, _currentToken.value.size() - 1)));
						_currentToken = _lexer.nextToken();
					}
				} else if (_currentToken.type == TOKEN_NUMBER) {
					server.setPort(std::stoi(_currentToken.value));
					_currentToken = _lexer.nextToken();	 // Consume port
				} else {
					reportError(LISTEN_MISSING_VALUES, "listen [host|port] or listen [host]:[port]", "listen [ ]");
				}

				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_SERVER_NAME:
				expect(TOKEN_SERVER_NAME);
				server.setServerName(_currentToken.value);
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_ROOT:
				expect(TOKEN_ROOT);
				server.setRoot(_currentToken.value);
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_INDEX:
				expect(TOKEN_INDEX);
				server.setIndex(_currentToken.value);
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_CLIENT_MAX_BODY_SIZE: {
				expect(TOKEN_CLIENT_MAX_BODY_SIZE);
				size_t maxBodySize = 0;
				size_t mbValue = std::stoul(_currentToken.value);
				expect(TOKEN_NUMBER);
				while (_currentToken.type == TOKEN_STRING) {
					switch (_currentToken.value[0]) {
						case 'k':
						case 'K':
							mbValue *= 1024;
							break;
						case 'm':
						case 'M':
							mbValue *= 1024 * 1024;
							break;
						case 'g':
						case 'G':
							mbValue *= 1024 * 1024 * 1024;
							break;
					}
					_currentToken = _lexer.nextToken();	 // Moves past the suffix

					if (_currentToken.type != TOKEN_NUMBER)
						break;
					maxBodySize += mbValue;
					mbValue = std::stoul(_currentToken.value);

					_currentToken = _lexer.nextToken();	 // Move past the number
				}
				maxBodySize += mbValue;
				server.setClientMaxBodySize(maxBodySize);
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_CLIENT_BODY_BUFFER_SIZE: {
				expect(TOKEN_CLIENT_BODY_BUFFER_SIZE);
				size_t bodyBufferSize = 0;
				size_t mbValue = std::stoul(_currentToken.value);
				expect(TOKEN_NUMBER);
				while (_currentToken.type == TOKEN_STRING) {
					switch (_currentToken.value[0]) {
						case 'k':
						case 'K':
							mbValue *= 1024;
							break;
						case 'm':
						case 'M':
							mbValue *= 1024 * 1024;
							break;
						case 'g':
						case 'G':
							mbValue *= 1024 * 1024 * 1024;
							break;
					}
					_currentToken = _lexer.nextToken();	 // Moves past the suffix

					if (_currentToken.type != TOKEN_NUMBER)
						break;
					bodyBufferSize += mbValue;
					mbValue = std::stoul(_currentToken.value);

					_currentToken = _lexer.nextToken();	 // Move past the number
				}
				bodyBufferSize += mbValue;
				server.setClientBodyBufferSize(bodyBufferSize);
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_CLIENT_HEADER_BUFFER_SIZE: {
				expect(TOKEN_CLIENT_HEADER_BUFFER_SIZE);
				size_t headerBufferSize = 0;
				size_t mbValue = std::stoul(_currentToken.value);
				expect(TOKEN_NUMBER);
				while (_currentToken.type == TOKEN_STRING) {
					switch (_currentToken.value[0]) {
						case 'k':
						case 'K':
							mbValue *= 1024;
							break;
						case 'm':
						case 'M':
							mbValue *= 1024 * 1024;
							break;
						case 'g':
						case 'G':
							mbValue *= 1024 * 1024 * 1024;
							break;
					}
					_currentToken = _lexer.nextToken();	 // Moves past the suffix

					if (_currentToken.type != TOKEN_NUMBER)
						break;
					headerBufferSize += mbValue;
					mbValue = std::stoul(_currentToken.value);

					_currentToken = _lexer.nextToken();	 // Move past the number
				}
				headerBufferSize += mbValue;
				server.setClientHeaderBufferSize(headerBufferSize);
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_UPLOAD_DIR:
				expect(TOKEN_UPLOAD_DIR);
				server.setUploadDir(_currentToken.value);
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_REQUEST_TIMEOUT: {
				expect(TOKEN_REQUEST_TIMEOUT);
				size_t timeout = 0;
				size_t msValue = 1000 * std::stoul(_currentToken.value);  // By default, read in seconds
				expect(TOKEN_NUMBER);
				while (_currentToken.type == TOKEN_STRING) {
					if (_currentToken.value == "ms")  // 1/1000th of a second
						msValue /= 1000;
					else if (_currentToken.value == "m")  // 60 seconds
						msValue *= 60;
					else if (_currentToken.value == "h")  // 60 minutes
						msValue *= 60 * 60;
					else if (_currentToken.value == "d")  // 24 hours
						msValue *= 24 * 60 * 60;
					else if (_currentToken.value == "w")  // 7 days
						msValue *= 7 * 24 * 60 * 60;
					else if (_currentToken.value == "M")  // 30 days
						msValue *= 30 * 24 * 60 * 60;
					else if (_currentToken.value == "y")  // 365 days
						msValue *= 365 * 24 * 60 * 60;

					_currentToken = _lexer.nextToken();	 // Moves past the suffix

					if (_currentToken.type != TOKEN_NUMBER)
						break;
					timeout += msValue;
					msValue = 1000 * std::stoul(_currentToken.value);

					_currentToken = _lexer.nextToken();	 // Move past the number
				}
				timeout += msValue;
				server.setRequestTimeout(timeout);
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_ERROR_PAGE: {
				expect(TOKEN_ERROR_PAGE);
				int code = std::stoi(_currentToken.value);
				expect(TOKEN_NUMBER);
				std::string path = _currentToken.value;
				expect(TOKEN_STRING);
				auto errorPages = server.getErrorPages();
				errorPages[code] = path;
				server.setErrorPages(errorPages);
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_LOCATION: {
				std::vector<Route> routes = server.getRoutes();
				routes.push_back(parseRoute());
				server.setRoutes(routes);
				break;
			}

			default:
				_currentToken = _lexer.nextToken();
				break;
				// throw std::runtime_error("Unexpected token in server body");
		}
	}

	expect(TOKEN_CLOSE_BRACE);
	return server;
}

Route Parser::parseRoute() {
	expect(TOKEN_LOCATION);
	Route route;
	route.setPath(_currentToken.value);
	expect(TOKEN_STRING);
	expect(TOKEN_OPEN_BRACE);

	while (_currentToken.type != TOKEN_CLOSE_BRACE) {
		switch (_currentToken.type) {
			case TOKEN_ALLOW_METHODS:
				expect(TOKEN_ALLOW_METHODS);
				{
					std::vector<std::string> methods;
					while (_currentToken.type == TOKEN_STRING) {
						methods.push_back(_currentToken.value);
						_currentToken = _lexer.nextToken();
					}
					route.setMethods(methods);
				}
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_ALIAS:
				expect(TOKEN_ALIAS);
				route.setAlias(_currentToken.value);
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_AUTOINDEX:
				expect(TOKEN_AUTOINDEX);
				if (_currentToken.type == TOKEN_ON) {
					route.setAutoindex(true);
				} else if (_currentToken.type == TOKEN_OFF) {
					route.setAutoindex(false);
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
				auto cgiHandlers = route.getCgiHandlers();
				cgiHandlers[ext] = handler;
				route.setCgiHandlers(cgiHandlers);
				expect(TOKEN_SEMICOLON);
				break;
			}

			case TOKEN_ROOT:
				expect(TOKEN_ROOT);
				route.setRoot(_currentToken.value);
				expect(TOKEN_STRING);
				expect(TOKEN_SEMICOLON);
				break;

			case TOKEN_RETURN: {
				expect(TOKEN_RETURN);

				if (_currentToken.type == TOKEN_NUMBER)
					route.setCode(std::stoi(_currentToken.value));
				else
					route.setCode(302);

				_currentToken = _lexer.nextTokenWhitespace();
				if (_currentToken.type == TOKEN_STRING) {
					route.setRedirect(_currentToken.value.substr(1, _currentToken.value.size() - 1));
					_currentToken = _lexer.nextToken();
					expect(TOKEN_SEMICOLON);
				} else if (_currentToken.type == TOKEN_SEMICOLON) {
					_currentToken = _lexer.nextToken();
					break;
				} else {
					throw std::runtime_error("Unexpected token");
				}

				break;
			}

			default:
				throw std::runtime_error("Unexpected token in route body");
		}
	}

	expect(TOKEN_CLOSE_BRACE);
	return route;
}

void Parser::reportError(eParsingErrors error, std::string expected, std::string found) {
	std::ostringstream errorMsg;

	errorMsg << COLOR(BLUE, _lexer.getErrorPrefix()) << COLOR(RED, parsingErrorsMessages.at(error).at(ERROR_NAME))
			 << std::endl
			 << std::endl
			 << std::left << std::setw(12) << parsingErrorsMessages.at(error).at(ERROR_TEXT);

	switch (error) {
		case UNEXPECTED_TOKEN:
			errorMsg << "'" << expected << "'";
			break;

		default:
			errorMsg << expected;
			break;
	}

	errorMsg << std::endl << std::left << std::setw(12) << "got: " << found << std::endl;

	_parsingErrors.push_back(errorMsg.str());
}

void Parser::flushErrors() const {
	for (const auto& msg : _parsingErrors) std::cerr << msg << std::endl << std::endl;
}
