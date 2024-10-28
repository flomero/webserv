/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 17:16:09 by flfische          #+#    #+#             */
/*   Updated: 2024/10/28 15:20:35 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

#include "webserv.hpp"

std::string readFile(const std::string &filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int main(int argc, char const *argv[]) {
	LOG_INFO("Starting server...");

	if (argc == 2) {
		std::string source;

		try {
			source = readFile(argv[1]);
		} catch (const std::runtime_error &e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}

		Lexer lexer(std::string(argv[1]), source);
		Parser parser(lexer);
		std::vector<ServerConfig> servers_config;

		try {
			servers_config = parser.parse();
		} catch (std::exception &e) {
			parser.flushErrors();
		}

		for (auto serv : servers_config) {
			std::cout << serv << std::endl;
		}

		std::cout << COLOR(YELLOW, "                     ~~~~~~~~ GET request TEST ~~~~~~~~") << std::endl << std::endl;

		std::string getRequestWithQueryString =
			"GET /test/scripts/test.py?size=large&format=png HTTP/1.1\n"
			"Host: www.example.com\n"
			"User-Agent: curl/7.68.0\n"
			"Accept: */*\n"
			"Connection: keep-alive\n"
			"Cache-Control: no-cache\n"
			"\r\n";	 // End of headers (denoted by empty line)
		HttpRequest getRequestWithQuery(getRequestWithQueryString);

		LOG_INFO(getRequestWithQueryString);
		servers_config.at(0).handleRequest(getRequestWithQuery);

		std::cout << std::endl << std::endl << std::endl << std::endl;

		std::cout << COLOR(YELLOW, "                     ~~~~~~~~ POST request TEST ~~~~~~~~") << std::endl
				  << std::endl;

		std::string postRequestWithBodyString =
			"POST /test/scripts/test.py HTTP/1.1\n"
			"Host: www.example.com\n"
			"User-Agent: curl/7.68.0\n"
			"Content-Type: application/x-www-form-urlencoded\n"
			"Content-Length: 34\n"
			"Connection: keep-alive\n"
			"\r\n"								   // End of headers
			"name=John+Doe&age=30&city=New+York";  // Body content
		HttpRequest postRequestWithBody(postRequestWithBodyString);

		LOG_INFO(postRequestWithBodyString);
		servers_config.at(0).handleRequest(postRequestWithBody);
	}

	return 0;
}