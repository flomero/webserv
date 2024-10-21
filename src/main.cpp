/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 17:16:09 by flfische          #+#    #+#             */
/*   Updated: 2024/10/21 17:15:46 by lgreau           ###   ########.fr       */
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
		std::vector<Server> servers_config;

		try {
			servers_config = parser.parse();
		} catch (std::exception &e) {
			parser.flushErrors();
		}

		for (auto serv : servers_config) {
			std::cout << serv << std::endl;
		}

		std::string getRequestWithQueryString =
			"GET /test/scripts/test.py?size=large&format=png HTTP/1.1\n"
			"Host: www.example.com\n"
			"User-Agent: curl/7.68.0\n"
			"Accept: */*\n"
			"Connection: keep-alive\n"
			"Cache-Control: no-cache\n"
			"\r\n";  // End of headers (denoted by empty line)
		HttpRequest getRequestWithQuery(getRequestWithQueryString);


		// std::string postRequestWithBodyString =
		// 	"POST /submit-form HTTP/1.1\n"
		// 	"Host: www.example.com\n"
		// 	"User-Agent: curl/7.68.0\n"
		// 	"Content-Type: application/x-www-form-urlencoded\n"
		// 	"Content-Length: 29\n"
		// 	"Connection: keep-alive\n"
		// 	"\r\n"  // End of headers
		// 	"name=John+Doe&age=30&city=New+York";  // Body content
		// HttpRequest postRequestWithBody(postRequestWithBodyString);


		LOG_INFO("Sending to " + servers_config.at(0).getServerName() + ":");
		LOG_INFO(getRequestWithQueryString);
		servers_config.at(0).handleRequest(getRequestWithQuery);
		// LOG_INFO(postRequestWithBodyString);
		// servers_config.at(0).handleRequest(postRequestWithBody);
	}

	return 0;
}