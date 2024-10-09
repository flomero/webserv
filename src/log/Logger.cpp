/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 14:20:32 by flfische          #+#    #+#             */
/*   Updated: 2024/10/09 15:09:36 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

Logger& Logger::getInstance() {
	static Logger instance;
	return instance;
}

void Logger::setOutputToFile(const std::string& filename) {
	logFile.open(filename, std::ios::app);
	if (logFile.is_open()) {
		outputToFile = true;
	} else {
		std::cerr << "Error: Cannot open log file: " << filename << std::endl;
	}
}

void Logger::setOutputToConsole() {
	outputToFile = false;
	if (logFile.is_open())
		logFile.close();
}

std::string Logger::getTimestamp() {
	std::time_t now = std::time(nullptr);
	std::tm* timeinfo = std::localtime(&now);
	char buffer[80];
	std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
	return std::string(buffer);
}

void Logger::log(const std::string& msg, LogLevel level) {
	if (outputToFile) {
		logFile << std::left << std::setw(20) << getTimestamp() << " "
				<< std::left << std::setw(10) << level << " " << msg
				<< std::endl;
	} else {
		std::cout << std::left << std::setw(20) << getTimestamp() << " "
				  << std::left << std::setw(20) << level << " " << msg
				  << std::endl;
	}
}

std::ostream& operator<<(std::ostream& os, LogLevel level) {
	switch (level) {
		case DEBUG:
			os << DEBUG_PREFIX;
			break;
		case INFO:
			os << INFO_PREFIX;
			break;
		case WARN:
			os << WARN_PREFIX;
			break;
		case ERROR:
			os << ERROR_PREFIX;
			break;
	}
	return os;
}
