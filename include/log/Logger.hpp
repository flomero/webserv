/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 14:14:03 by flfische          #+#    #+#             */
/*   Updated: 2024/10/15 16:21:42 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ft_iomanip.hpp"

#define DEBUG_COLOR BLUE
#define INFO_COLOR CYAN
#define WARN_COLOR YELLOW
#define ERROR_COLOR RED

#define DEBUG_PREFIX DEBUG_COLOR "[DEBUG]" RESET_COLOR
#define INFO_PREFIX INFO_COLOR "[INFO]" RESET_COLOR
#define WARN_PREFIX WARN_COLOR "[WARN]" RESET_COLOR
#define ERROR_PREFIX ERROR_COLOR "[ERROR]" RESET_COLOR

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
};

std::ostream& operator<<(std::ostream& os, LogLevel level);

#define LOG_DEBUG(msg) Logger::getInstance().log(msg, DEBUG)
#define LOG_INFO(msg) Logger::getInstance().log(msg, INFO)
#define LOG_WARN(msg) Logger::getInstance().log(msg, WARN)
#define LOG_ERROR(msg) Logger::getInstance().log(msg, ERROR)