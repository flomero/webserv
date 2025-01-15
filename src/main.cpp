/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 17:16:09 by flfische          #+#    #+#             */
/*   Updated: 2025/01/14 12:36:30 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

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

void validateServerConfigs(const std::vector<std::vector<ServerConfig>> &server_config_vectors) {
	// At least one server configuration vector
	if (server_config_vectors.empty())
		throw std::runtime_error("At least one server configuration vector has to be created");

	// At least one server in each configuration vector
	for (const auto &server_configs : server_config_vectors) {
		if (server_configs.empty())
			throw std::runtime_error("Each server configuration vector must contain at least one server");
	}
}

void printServerConfigs(const std::vector<std::vector<ServerConfig>>& server_config_vectors) {
	for (const auto& server_configs : server_config_vectors) {
		for (const auto& config : server_configs) {
			std::cout << config << std::endl;
		}
	}
}

int main(const int argc, const char *argv[]) {
	if (argc != 2) {
		std::cerr << COLOR(RED, "Error: ") << "Invalid number of arguments" << std::endl;
		std::cerr << "Usage: " << argv[0] << " <path_to_config_file>" << std::endl;
		return 1;
	}

	std::string source;
	std::vector<std::vector<ServerConfig>> server_config_vectors;

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
		server_config_vectors = parser.parse();
		validateServerConfigs(server_config_vectors);
	} catch (...) {
		parser.flushErrors();
		return 1;
	}

	printServerConfigs(server_config_vectors);

	try {
		LOG_INFO("Starting server...");
		MultiSocketWebserver server(server_config_vectors);
		server.initSockets();
		server.run();
	} catch (const std::exception &e) {
		LOG_ERROR("Server Error: " + std::string(e.what()));
		return 1;
	}

	return 0;
}
