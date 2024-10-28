/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCGIExecution.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/21 15:55:29 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/28 16:11:45 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "RequestHandler.hpp"

void RequestHandler::handleRequestCGIExecution(Route& route) {
	std::string cgiPath = route.getCgiHandlers().at(_request.getRessourceExtension());

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

	// env["REMOTE_ADDR"] = request.getClientIP(); TODO

	// Prepare environment for execv
	LOG_INFO("Prepare environment for execv");
	char* envp[env.size() + 1];
	size_t i = 0;
	for (const auto& [key, value] : env) {
		std::string envVar = key + "=" + value;
		envp[i] = strdup(envVar.c_str());
		++i;
	}
	envp[i] = nullptr;

	int pipeIn[2], pipeOut[2];
	pipe(pipeIn);	// For input to the CGI script
	pipe(pipeOut);	// For output from the CGI script

	pid_t pid = fork();
	if (pid == 0) {
		// Child process: set up pipes and execv
		close(pipeIn[1]);	// Close write end of input pipe
		close(pipeOut[0]);	// Close read end of output pipe

		dup2(pipeIn[0], STDIN_FILENO);	  // Redirect stdin to pipeIn
		dup2(pipeOut[1], STDOUT_FILENO);  // Redirect stdout to pipeOut

		// Execute CGI
		char* argv[] = {strdup(cgiPath.c_str()), strdup(_request.getServerSidePath().c_str()), nullptr};
		if (execve(argv[0], argv, envp) == -1) {
			// Print error message if execve fails
			perror("execve failed");
			exit(EXIT_FAILURE);	 // Exit if exec fails
		}

		LOG_ERROR("Child process exec failed");

		// If exec fails, exit
		exit(1);
	} else {
		// Parent process: send data to child and read response
		close(pipeIn[0]);	// Close read end of input pipe
		close(pipeOut[1]);	// Close write end of output pipe

		// Write POST data to the CGI process if it's a POST request
		if (_request.getMethod() == "POST")
			write(pipeIn[1], _request.getBody().c_str(), _request.getBody().size());

		close(pipeIn[1]);  // Close write end of input pipe

		// Read the CGI output
		char buffer[4096];
		ssize_t bytesRead;
		std::string response;
		while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0) response.append(buffer, bytesRead);

		close(pipeOut[0]);
		LOG_DEBUG("response:\n" + response + "\n");

		// Process the response from the CGI script
		// Here, you need to parse headers and body
		// Check if headers were found, otherwise handle error
		size_t headerEnd = response.find("\r\n\r\n");
		if (headerEnd != std::string::npos) {
			std::string headers = response.substr(0, headerEnd);
			std::string body = response.substr(headerEnd + 4);	// Body starts after the "\r\n\r\n"

			LOG_DEBUG("Headers: " + headers);
			LOG_DEBUG("Body: " + body);

			// Send headers and body to the client
			std::cout << "Headers: " << headers << std::endl;
			std::cout << "Body: " << body << std::endl;
		} else {
			LOG_ERROR("No headers found in the response.");
		}

		// Wait for the child process to finish
		waitpid(pid, nullptr, 0);
	}
}
