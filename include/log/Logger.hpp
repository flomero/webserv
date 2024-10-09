/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 14:14:03 by flfische          #+#    #+#             */
/*   Updated: 2024/10/09 15:11:54 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

#define DEBUG_COLOR BLUE
#define INFO_COLOR CYAN
#define WARN_COLOR YELLOW
#define ERROR_COLOR RED

#define DEBUG_PREFIX DEBUG_COLOR "[DEBUG]" RESET
#define INFO_PREFIX INFO_COLOR "[INFO]" RESET
#define WARN_PREFIX WARN_COLOR "[WARN]" RESET
#define ERROR_PREFIX ERROR_COLOR "[ERROR]" RESET

enum LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
	public:
		Logger() = default;
		~Logger() = default;
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		static Logger& getInstance();

		void log(const std::string& msg, LogLevel level);

		void setOutputToFile(const std::string& filename);
		void setOutputToConsole();

	private:
		bool outputToFile = false;
		std::ofstream logFile;

		std::ostringstream formatMessage(const std::string& msg,
										 LogLevel level);
		std::string getTimestamp();
};

std::ostream& operator<<(std::ostream& os, LogLevel level);

#define LOG_DEBUG(msg) Logger::getInstance().log(msg, DEBUG)
#define LOG_INFO(msg) Logger::getInstance().log(msg, INFO)
#define LOG_WARN(msg) Logger::getInstance().log(msg, WARN)
#define LOG_ERROR(msg) Logger::getInstance().log(msg, ERROR)