/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:15:21 by flfische          #+#    #+#             */
/*   Updated: 2025/01/15 21:16:39 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <unordered_set>
#include <vector>

#include "HttpMessage.hpp"

/**
 * @brief Represents an HTTP request
 */
class HttpRequest : public HttpMessage {
	public:
		enum class BodyType { NO_BODY, CHUNKED, CONTENT_LENGTH };

		HttpRequest() = default;
		explicit HttpRequest(const std::string &rawRequest);
		~HttpRequest() override = default;

		// Getters
		[[nodiscard]] std::string getMethod() const;
		[[nodiscard]] std::string getRequestUri() const;
		[[nodiscard]] std::string getServerSidePath() const;
		[[nodiscard]] bool getIsFile() const;
		[[nodiscard]] std::string getResourceExtension() const;
		[[nodiscard]] std::string getQueryString() const;
		[[nodiscard]] std::string getLocation() const;

		[[nodiscard]] BodyType getBodyType() const { return _bodyType; }

		[[nodiscard]] size_t getContentLength() const { return _contentLength; }

		// Setters
		void setMethod(const std::string &method);
		void setRequestUri(const std::string &requestUri);
		void setServerSidePath(const std::string &serverSidePath);
		void setIsFile(bool isFile);
		void setResourceExtension(const std::string &resourceExtension);
		void setQueryString(const std::string &queryString);
		void setLocation(const std::string &location);

		// Error 400
		class BadRequest final : public std::exception {
			public:
				[[nodiscard]] const char *what() const noexcept override {
					return "Bad request - invalid HTTP request line";
				}
		};

		// Error 501
		class NotImplemented final : public std::exception {
			public:
				[[nodiscard]] const char *what() const noexcept override { return "Method not implemented"; }
		};

		// Error 505
		class InvalidVersion final : public std::exception {
			public:
				[[nodiscard]] const char *what() const noexcept override { return "HTTP version not supported"; }
		};

	private:
		std::string _method;
		std::string _requestUri;
		std::string _rawRequest;
		std::string _serverSidePath;
		bool _isFile{};
		std::string _location;
		BodyType _bodyType;
		std::string _resourceExtension;
		std::string _queryString;
		size_t _contentLength = 0;

		void _parseURI();
		void _decodeURL();
		void _validateRequestLine() const;
		void _validateHeaders() const;
		void _initBodyType();

		std::unordered_set<std::string> _supportedMethods = {"GET", "POST", "DELETE"};
		std::unordered_set<std::string> _unsupportedMethods = {"PUT", "HEAD", "OPTIONS", "TRACE", "CONNECT", "PATCH"};
		std::unordered_set<std::string> _supportedVersions = {"HTTP/1.0", "HTTP/1.1"};
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);
