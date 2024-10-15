/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 17:16:09 by flfische          #+#    #+#             */
/*   Updated: 2024/10/15 16:45:05 by lgreau           ###   ########.fr       */
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

		std::string exampleRequest = "GET test/upload/icons/icon.png HTTP/1.1\nHost: www.example.com\nUser-Agent: curl/7.68.0\nAccept: */*";
		HttpRequest testRequest(exampleRequest);
		servers_config.at(0).handleRequest(testRequest);
	}

	return 0;
}