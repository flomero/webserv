/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 17:16:09 by flfische          #+#    #+#             */
/*   Updated: 2025/01/19 11:57:24 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <atomic>
#include <csignal>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

#include "MultiSocketWebserver.hpp"
#include "globals.hpp"
#include "webserv.hpp"

std::atomic<bool> stopServer(false);

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

void printServerConfigs(const std::vector<std::vector<ServerConfig>> &server_config_vectors) {
	for (const auto &server_configs : server_config_vectors) {
		for (const auto &config : server_configs) {
			std::cout << config << std::endl;
		}
	}
}

// Signal handler function
void signalHandler(const int signum) {
	if (stopServer) {
		LOG_INFO("Server is already stopping...");
		return;
	}
	// Clear the line using ANSI escape code
	std::cout << "\033[2K\r";
	LOG_INFO("Interrupt signal (" + std::to_string(signum) + ") received. Stopping server...");
	stopServer = true;

	// Ignore further SIGINT signals to prevent ^C from being printed
	signal(SIGINT, SIG_IGN);
}

int main(const int argc, const char *argv[]) {
	std::string filepath;
	if (argc != 2) {
		LOG_WARN("No configuration file provided. Using default configuration file: " + DEFAULT_CONFIG_STR);
		LOG_WARN("Usage: " + std::string(argv[0]) + " <config_file>");
		filepath = DEFAULT_CONFIG_STR;
	} else {
		filepath = argv[1];
	}

	// Register signal handler
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	std::string source;
	std::vector<std::vector<ServerConfig>> server_config_vectors;

	LOG_INFO("Parsing configuration file...");
	try {
		source = readFile(filepath);
	} catch (const std::runtime_error &e) {
		LOG_ERROR("Failed to read configuration file: " + std::string(e.what()));
		return 1;
	}

	Lexer lexer(filepath, source);
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
