/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 17:16:09 by flfische          #+#    #+#             */
/*   Updated: 2024/10/28 16:40:08 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <poll.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

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
		for (const auto &serv : servers_config) {
			std::cout << serv << std::endl;
		}

		std::vector<Socket> sockets;
		std::vector<pollfd> pollFds;
		std::unordered_set<int> serverSockets;	// Track which fds are server sockets

		sockets.reserve(servers_config.size());
		for (const auto &serv : servers_config) {
			sockets.emplace_back(serv.getPort());
			int server_fd = sockets.back().getSocketFd();
			pollFds.emplace_back(pollfd{server_fd, POLLIN, 0});
			serverSockets.insert(server_fd);  // Mark as a server socket
		}

		while (true) {
			int eventCount = poll(pollFds.data(), pollFds.size(), -1);
			if (eventCount == -1) {
				perror("Poll failed");
				return 1;
			}

			for (size_t i = 0; i < pollFds.size(); ++i) {
				int fd = pollFds[i].fd;
				short revents = pollFds[i].revents;

				if (revents & POLLIN) {
					if (serverSockets.count(fd)) {
						// Handle new incoming connection on server socket
						sockaddr_in clientAddr;
						socklen_t addrLen = sizeof(clientAddr);
						int clientFd = ::accept(fd, reinterpret_cast<sockaddr *>(&clientAddr), &addrLen);

						if (clientFd == -1) {
							LOG_ERROR("Accept failed: " + std::string(strerror(errno)));
							continue;
						}

						if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
							LOG_ERROR("Failed to set client socket to non-blocking: " + std::string(strerror(errno)));
							close(clientFd);
							continue;  // Handle error and continue
						}

						pollFds.emplace_back(pollfd{clientFd, POLLIN, 0});
						LOG_INFO("Accepted connection from " + std::string(inet_ntoa(clientAddr.sin_addr)) +
								 " on socket " + std::to_string(clientFd));

					} else {
						// Handle data from a client socket
						char buffer[4096];
						ssize_t bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
						if (bytesReceived <= 0) {
							if (bytesReceived == 0 || (bytesReceived == -1 && errno != EAGAIN)) {
								close(fd);
								pollFds.erase(pollFds.begin() + i);
								--i;
							}
							continue;
						}

						std::string request(buffer, bytesReceived);
						LOG_INFO("Received request: " + request);

						std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
						send(fd, response.c_str(), response.size(), 0);

						close(fd);
						pollFds.erase(pollFds.begin() + i);
						--i;
					}
				}
			}
		}

		// pollFds[0].revents = POLLIN;

		// std::cout << COLOR(YELLOW, "                     ~~~~~~~~ GET request TEST ~~~~~~~~") << std::endl <<
		// std::endl;
		//
		// std::string getRequestWithQueryString =
		// 	"GET /test/scripts/test.py?size=large&format=png HTTP/1.1\n"
		// 	"Host: www.example.com\n"
		// 	"User-Agent: curl/7.68.0\n"
		// 	"Accept: */*\n"
		// 	"Connection: keep-alive\n"
		// 	"Cache-Control: no-cache\n"
		// 	"\r\n";	 // End of headers (denoted by empty line)
		//
		// LOG_INFO(getRequestWithQueryString);
		// RequestHandler handler(servers_config.at(0));
		// handler.handleRequest(getRequestWithQueryString);
		//
		// std::cout << std::endl << std::endl << std::endl << std::endl;
		//
		// std::cout << COLOR(YELLOW, "                     ~~~~~~~~ POST request TEST ~~~~~~~~") << std::endl
		// 		  << std::endl;
		//
		// std::string postRequestWithBodyString =
		// 	"POST /test/scripts/test.py HTTP/1.1\n"
		// 	"Host: www.example.com\n"
		// 	"User-Agent: curl/7.68.0\n"
		// 	"Content-Type: application/x-www-form-urlencoded\n"
		// 	"Content-Length: 34\n"
		// 	"Connection: keep-alive\n"
		// 	"\r\n"								   // End of headers
		// 	"name=John+Doe&age=30&city=New+York";  // Body content
		//
		// LOG_INFO(postRequestWithBodyString);
		// RequestHandler handler2(servers_config.at(0));
		// handler2.handleRequest(postRequestWithBodyString);
	}

	return 0;
}
