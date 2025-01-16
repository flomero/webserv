/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCGIExecution.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/21 15:55:29 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/16 14:37:13 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <cstring>
#include <thread>

#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "Route.hpp"

void RequestHandler::handleRequestCGIExecution(const Route& route) {
	const std::string cgiPath = route.getCgiHandlers().at(_request.getResourceExtension());

	// Create environment variables for CGI
	LOG_INFO("Create environment variables for CGI");
	std::map<std::string, std::string> env;
	env["REQUEST_METHOD"] = _request.getMethod();
	env["QUERY_STRING"] = _request.getQueryString();
	env["SCRIPT_NAME"] = _request.getServerSidePath();
	env["PATH_INFO"] = _request.getServerSidePath();
	env["CONTENT_LENGTH"] = std::to_string(_request.getBody().size());
	env["CONTENT_TYPE"] = _request.getHeader("Content-Type");

	for (const auto& [key, value] : _request.getHeaders()) {
		std::string headerKey = "HTTP_" + key;
		std::transform(headerKey.begin(), headerKey.end(), headerKey.begin(), ::toupper);
		std::replace(headerKey.begin(), headerKey.end(), '-', '_');
		env[headerKey] = value;
	}

	// Prepare environment for execv
	LOG_INFO("Prepare environment for execv");
	std::vector<std::string> envStrings;
	std::vector<char*> envp;
	for (const auto& [key, value] : env) {
		envStrings.push_back(key + "=" + value);
		envp.push_back(envStrings.back().data());
	}
	envp.push_back(nullptr);

	int pipeIn[2], pipeOut[2];
	pipe(pipeIn);
	pipe(pipeOut);

	const pid_t pid = fork();
	if (pid == 0) {
		// Child process: set up pipes and execv
		close(pipeIn[1]);
		close(pipeOut[0]);

		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);

		std::string script_path = _request.getServerSidePath();
		size_t sep = script_path.find_last_of('/');
		if (sep != std::string::npos) {
			std::string cd_path = script_path.substr(0, sep);
			script_path = script_path.substr(sep + 1);
			if (chdir(cd_path.c_str()) != 0)
				LOG_ERROR("chdir error");
		}

		std::vector<std::string> args = {cgiPath, script_path};
		std::vector<char*> argv;
		for (auto& arg : args) {
			argv.push_back(arg.data());
		}
		argv.push_back(nullptr);

		execve(argv[0], argv.data(), envp.data());
		perror("execve failed");
		exit(EXIT_FAILURE);
	}

	// Parent process: send data to child and read response
	close(pipeIn[0]);
	close(pipeOut[1]);

	if (_request.getMethod() == "POST") {
		write(pipeIn[1], _request.getBody().c_str(), _request.getBody().size());
	}
	close(pipeIn[1]);

	auto start_time = std::chrono::high_resolution_clock::now();
	int status;
	bool child_terminated = false;

	while (true) {
		pid_t result = waitpid(pid, &status, WNOHANG);
		if (result == pid) {
			child_terminated = true;
			break;
		}

		auto now = std::chrono::high_resolution_clock::now();
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
		if (elapsed_ms > DEFAULT_CGI_TIMEOUT_MS) {
			LOG_ERROR("CGI execution timed out. Killing process...");
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	char buffer[4096];
	ssize_t bytesRead;
	std::string response;

	if (child_terminated) {
		while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0) {
			response.append(buffer, bytesRead);
		}
		_response = HttpResponse(response);
		_response.setStatus(status == 0 ? Http::OK : Http::INTERNAL_SERVER_ERROR);
	} else {
		_response = buildDefaultResponse(Http::GATEWAY_TIMEOUT);
	}
	_cgiExecuted = true;
	close(pipeOut[0]);
}
