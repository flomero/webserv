/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:43:36 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/06 14:07:09 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/webserv.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>

std::string readFile(const std::string &filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int main(int argc, char const *argv[])
{
	if (argc == 2) {
		std::string source;

		try {
			source = readFile(argv[1]);
		} catch (const std::runtime_error &e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}

		Lexer lexer(source);
		Parser parser(lexer);

		std::vector<Server> servers_config = parser.parse();

		for (auto serv : servers_config) {
			std::cout	<< serv
						<< std::endl;
		}
	}

	return 0;
}
