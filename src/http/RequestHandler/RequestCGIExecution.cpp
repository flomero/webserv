/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCGIExecution.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/21 15:55:29 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/15 17:22:42 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <unistd.h>

#include <csignal>

#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "Route.hpp"
#include "thread"

void RequestHandler::handleRequestCGIExecution(const Route& route) {
	const std::string cgiPath = route.getCgiHandlers().at(_request.getResourceExtension());

	// Create environment variables for CGI
	LOG_INFO("Create environment variables for CGI");
	std::map<std::string, std::string> env;
	env["REQUEST_METHOD"] = _request.getMethod();
	LOG_DEBUG("  |- REQUEST_METHOD:    " + env["REQUEST_METHOD"]);

	env["QUERY_STRING"] = _request.getQueryString();  // for GET requests
	LOG_DEBUG("  |- QUERY_STRING:      " + env["QUERY_STRING"]);

	env["SCRIPT_NAME"] = _request.getServerSidePath();
	LOG_DEBUG("  |- SCRIPT_NAME:       " + env["SCRIPT_NAME"]);

	env["PATH_INFO"] = _request.getServerSidePath();
	LOG_DEBUG("  |- PATH_INFO:         " + env["PATH_INFO"]);

	env["CONTENT_LENGTH"] = std::to_string(_request.getBody().size());
	LOG_DEBUG("  |- CONTENT_LENGTH:    " + env["CONTENT_LENGTH"]);

	env["CONTENT_TYPE"] = _request.getHeader("Content-Type");
	LOG_DEBUG("  |- CONTENT_TYPE:      " + env["CONTENT_TYPE"] + "\n");

	// Prepare environment for execv
	LOG_INFO("Prepare environment for execv");
	char* envp[env.size() + 1];
	size_t i = 0;
	for (const auto& [key, value] : env) {
		std::string envVar = key + "=";
		envVar += value;
		envp[i] = strdup(envVar.c_str());
		++i;
	}
	envp[i] = nullptr;

	int pipeIn[2], pipeOut[2];
	pipe(pipeIn);	// For input to the CGI script
	pipe(pipeOut);	// For output from the CGI script

	const pid_t pid = fork();
	if (pid == 0) {
		// Child process: set up pipes and execv
		close(pipeIn[1]);	// Close write end of input pipe
		close(pipeOut[0]);	// Close read end of output pipe

		dup2(pipeIn[0], STDIN_FILENO);	  // Redirect stdin to pipeIn
		dup2(pipeOut[1], STDOUT_FILENO);  // Redirect stdout to pipeOut

		std::string script_path = _request.getServerSidePath();
		if (_request.getServerSidePath().find('/') != _request.getServerSidePath().size()) {
			size_t sep = _request.getServerSidePath().find_last_of('/');
			std::string cd_path = script_path.substr(0, sep);
			LOG_DEBUG("cd_path: " + cd_path);

			script_path = _request.getServerSidePath().substr(sep + 1);
			LOG_DEBUG("script_path: " + script_path);

			if (chdir(cd_path.c_str()) != 0)
				LOG_ERROR("chdir error");
		}

		// Execute CGI
		if (char* argv[] = {strdup(cgiPath.c_str()), strdup(script_path.c_str()), nullptr};

			execve(argv[0], argv, envp) == -1) {
			// Print error message if execve fails
			perror("execve failed");
			exit(EXIT_FAILURE);	 // Exit if exec fails
		}

		LOG_ERROR("Child process exec failed");

		// If exec fails, exit
		exit(1);
	}

	// Parent process: send data to child and read response
	close(pipeIn[0]);	// Close read end of input pipe
	close(pipeOut[1]);	// Close write end of output pipe

	// Write POST data to the CGI process if it's a POST request
	if (_request.getMethod() == "POST")
		write(pipeIn[1], _request.getBody().c_str(), _request.getBody().size());

	close(pipeIn[1]);  // Close write end of input pipe

	// Timeout handling
	auto start_time = std::chrono::high_resolution_clock::now();
	int status;
	bool child_terminated = false;

	while (true) {
		// Check if the child process has terminated
		pid_t result = waitpid(pid, &status, WNOHANG);
		if (result == pid) {
			child_terminated = true;
			break;
		}

		// Check if timeout has been exceeded
		auto now = std::chrono::high_resolution_clock::now();
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
		if (elapsed_ms > DEFAULT_CGI_TIMEOUT_MS) {
			LOG_ERROR("CGI execution timed out. Killing process...");
			kill(pid, SIGKILL);		   // Kill the child process
			waitpid(pid, &status, 0);  // Wait for the process to be reaped
			break;
		}

		// Sleep briefly to avoid busy-waiting
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	char buffer[4096];
	ssize_t bytesRead;
	std::string response;

	if (child_terminated) {
		// Read the CGI output
		while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0) {
			response.append(buffer, bytesRead);
		}
		_response = HttpResponse(response);
		if (status == 0)
			_response.setStatus(Http::OK);
		else
			_response.setStatus(Http::INTERNAL_SERVER_ERROR);
		_cgiExecuted = true;
	} else {
		_response = buildDefaultResponse(Http::Status::GATEWAY_TIMEOUT);
		_cgiExecuted = true;
	}

	close(pipeOut[0]);
	LOG_DEBUG("response:\n" + response + "\n");
}
