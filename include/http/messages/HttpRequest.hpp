/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:15:21 by flfische          #+#    #+#             */
/*   Updated: 2024/10/21 16:15:20 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>

#include "HttpMessage.hpp"

/**
 * @brief Represents an HTTP request
 */
class HttpRequest : public HttpMessage {
	public:
		HttpRequest(const std::string &rawRequest);
		virtual ~HttpRequest() = default;

		// Getters
		[[nodiscard]] std::string getMethod() const;
		[[nodiscard]] std::string getRequestUri() const;
		[[nodiscard]] std::string getServerSidePath() const;
		[[nodiscard]] bool getIsFile() const;
		[[nodiscard]] std::string getRessourceExtension() const;
		[[nodiscard]] std::string getQueryString() const;
		[[nodiscard]] int getRequestLength() const;

		// Setters
		void setMethod(const std::string &method);
		void setRequestUri(const std::string &requestUri);
		void setServerSidePath(const std::string &serverSidePath);
		void setIsFile(bool isFile);
		void setRessourceExtension(const std::string &ressourceExtension);
		void setQueryString(const std::string &queryString);

		// Error 400
		class BadRequest : public std::exception {
			public:
				const char *what() const noexcept override {
					return "Bad request - invalid HTTP request line";
				}
		};

		// Error 501
		class NotImplemented : public std::exception {
			public:
				const char *what() const noexcept override {
					return "Method not implemented";
				}
		};

		// Error 505
		class InvalidVersion : public std::exception {
			public:
				const char *what() const noexcept override {
					return "HTTP version not supported";
				}
		};

	private:
		HttpRequest() = default;
		std::string _method;
		std::string _requestUri;
		std::string _rawRequest;
		std::string _serverSidePath;
		bool _isFile;
		std::string _ressourceExtension;
		std::string _queryString;

		void validate() const;
		void parseChunkedBody(std::istringstream &requestStream);

		static const std::vector<std::string> _supportedMethods;
		static const std::vector<std::string> _unsupportedMethods;
		static const std::vector<std::string> _supportedVersions;
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);
