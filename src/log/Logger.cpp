/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 14:20:32 by flfische          #+#    #+#             */
/*   Updated: 2024/10/09 15:21:54 by flfische         ###   ########.fr       */
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

void Logger::log(const std::string& msg, const LogLevel level) {
	const std::time_t now = std::time(nullptr);
	if (outputToFile) {
		logFile << std::left << std::put_time(std::localtime(&now), "%F %T") << " " << std::left << std::setw(20)
				<< level << " " << msg << std::endl;
	} else {
		std::cout << std::left << std::put_time(std::localtime(&now), "%F %T") << " " << std::left << std::setw(20)
				  << level << " " << msg << std::endl;
	}
}

std::ostream& operator<<(std::ostream& os, const LogLevel level) {
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
