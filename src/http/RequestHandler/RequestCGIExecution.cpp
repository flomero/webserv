/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCGIExecution.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/21 15:55:29 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/19 11:26:27 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <poll.h>
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
#include "webserv.hpp"

void RequestHandler::handleRequestCGIExecution(const Route& route) {
	_cgi_valid = true;
	if (!_cgi_state) {
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

		pipe(_cgi_pipeIn);
		pipe(_cgi_pipeOut);

		const pid_t pid = fork();
		if (pid == 0) {
			// Child process: set up pipes and execv
			close(_cgi_pipeIn[1]);
			close(_cgi_pipeOut[0]);

			dup2(_cgi_pipeIn[0], STDIN_FILENO);
			dup2(_cgi_pipeOut[1], STDOUT_FILENO);

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
		close(_cgi_pipeIn[0]);
		close(_cgi_pipeOut[1]);
		if (_request.getMethod() == "POST") {
			_cgi_state = cgiState::WRITING;
		} else {
			close(_cgi_pipeIn[1]);
			_cgi_state = cgiState::WAITING;
			_cgi_startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch());
		}
	}
	if (_cgi_state == cgiState::WRITING) {
		LOG_DEBUG("Writing to CGI process");

		size_t bytesToWrite = _request.getBody().size();
		size_t offset = 0;
		while (bytesToWrite > 0) {
			pollfd pfd{};
			pfd.fd = _cgi_pipeIn[1];
			pfd.events = POLLOUT;

			int pollret = poll(&pfd, 1, DEFAULT_POLL_TIMEOUT);
			if (pollret < 0) {
				LOG_ERROR("Poll error while writing to CGI process: " + std::string(strerror(errno)));
				close(_cgi_pipeIn[1]);
				_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
				_cgi_state = cgiState::FINISHED;
				return;
			}
			if (pollret == 0) {
				LOG_WARN("Poll timeout while writing to CGI process");
				close(_cgi_pipeIn[1]);
				_response = buildDefaultResponse(Http::GATEWAY_TIMEOUT);
				_cgi_state = cgiState::FINISHED;
				return;
			}
			if (pfd.revents & POLLOUT) {
				size_t chunkSize = std::min(POST_WRITE_SIZE, bytesToWrite);
				ssize_t written = write(_cgi_pipeIn[1], _request.getBody().c_str() + offset, chunkSize);
				if (written < 0) {
					LOG_ERROR("Write error to CGI process: " + std::string(strerror(errno)));
					close(_cgi_pipeIn[1]);
					_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
					_cgi_state = cgiState::FINISHED;
					return;
				}
				offset += written;
				bytesToWrite -= written;
			} else {
				LOG_WARN("Unexpected poll revents: " + std::to_string(pfd.revents));
				close(_cgi_pipeIn[1]);
				_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
				_cgi_state = cgiState::FINISHED;
				return;
			}
		}
		close(_cgi_pipeIn[1]);
		_cgi_state = cgiState::WAITING;
		_cgi_startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch());
	}
	if (_cgi_state == cgiState::WAITING) {
		LOG_TRACE("Waiting for CGI process to finish");
		pid_t result = waitpid(_cgi_pid, &_cgi_status, WNOHANG);
		if (result != _cgi_pid) {
			_cgi_state = cgiState::READING;
		} else {
			auto now = std::chrono::high_resolution_clock::now();
			auto elapsed_ms =
				std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() - _cgi_startTime).count();
			if (elapsed_ms > DEFAULT_CGI_TIMEOUT_MS) {
				LOG_ERROR("CGI execution timed out. Killing process...");
				kill(_cgi_pid, SIGKILL);
				waitpid(_cgi_pid, &_cgi_status, 0);	 // TODO: why is this needed?
				_response = buildDefaultResponse(Http::GATEWAY_TIMEOUT);
				_cgi_state = cgiState::FINISHED;
			}
		}
	}
	if (_cgi_state == cgiState::READING) {
		LOG_DEBUG("Reading from CGI process");
		char buffer[CGI_READ_BUFFER_SIZE];
		pollfd pfd{};
		pfd.fd = _cgi_pipeOut[0];
		pfd.events = POLLIN;

		int pollret = poll(&pfd, 1, DEFAULT_POLL_TIMEOUT);
		if (pollret < 0) {
			LOG_ERROR("Poll error while reading from CGI process: " + std::string(strerror(errno)));
			close(_cgi_pipeOut[0]);
			_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
			_cgi_state = cgiState::FINISHED;
			return;
		}
		if (pollret == 0) {
			LOG_WARN("Poll timeout while reading from CGI process");
			close(_cgi_pipeOut[0]);
			_response = buildDefaultResponse(Http::GATEWAY_TIMEOUT);
			_cgi_state = cgiState::FINISHED;
			return;
		}
		if (pfd.revents & POLLIN) {
			const ssize_t bytesRead = read(_cgi_pipeOut[0], buffer, sizeof(buffer));
			if (bytesRead > 0) {
				_response.appendToBody(std::string(buffer, bytesRead));
			} else if (bytesRead == 0) {
				_response = HttpResponse(_response.getBody());
				_response.setStatus(_cgi_status == 0 ? Http::OK : Http::INTERNAL_SERVER_ERROR);
				close(_cgi_pipeOut[0]);
				_cgi_state = cgiState::FINISHED;
			} else {
				LOG_ERROR("Error reading from CGI process");
				_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
				_cgi_state = cgiState::FINISHED;
			}
		} else {
			LOG_WARN("Unexpected poll revents: " + std::to_string(pfd.revents));
			close(_cgi_pipeOut[0]);
			_response = buildDefaultResponse(Http::INTERNAL_SERVER_ERROR);
			_cgi_state = cgiState::FINISHED;
			return;
		}
	}
}