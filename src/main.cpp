/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 17:16:09 by flfische          #+#    #+#             */
/*   Updated: 2024/12/10 19:31:32 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <poll.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include "MultiSocketWebserver.hpp"
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
	if (argc != 2) {
		std::cerr << COLOR(RED, "Error: ") << "Invalid number of arguments" << std::endl;
		std::cerr << "Usage: " << argv[0] << " <path_to_config_file>" << std::endl;
		return 1;
	}

	std::string source;
	std::vector<ServerConfig> servers_config;

	LOG_INFO("Parsing configuration file...");
	try {
		source = readFile(argv[1]);
	} catch (const std::runtime_error &e) {
		LOG_ERROR("Failed to read configuration file: " + std::string(e.what()));
		return 1;
	}

	Lexer lexer(std::string(argv[1]), source);
	Parser parser(lexer);

	try {
		servers_config = parser.parse();
	} catch (std::exception &e) {
		parser.flushErrors();
	}
	for (const auto &serv : servers_config) {
		std::cout << serv << std::endl;
	}

	try {
		LOG_INFO("Starting server...");
		MultiSocketWebserver server(servers_config);
		server.initSockets();
		server.run();
	} catch (const std::exception &e) {
		LOG_ERROR("Server Error: " + std::string(e.what()));
		return 1;
	}

	return 0;
}
