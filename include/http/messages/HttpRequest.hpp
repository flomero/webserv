/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:15:21 by flfische          #+#    #+#             */
/*   Updated: 2024/11/01 16:29:39 by flfische         ###   ########.fr       */
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
		enum class BodyType { NO_BODY, CHUNKED, CONTENT_LENGTH };

		HttpRequest() = default;
		HttpRequest(const std::string &rawRequest);
		virtual ~HttpRequest() = default;

		// Getters
		[[nodiscard]] std::string getMethod() const;
		[[nodiscard]] std::string getRequestUri() const;
		[[nodiscard]] std::string getServerSidePath() const;
		[[nodiscard]] bool getIsFile() const;
		[[nodiscard]] std::string getRessourceExtension() const;
		[[nodiscard]] std::string getQueryString() const;
		[[nodiscard]] std::string getLocation() const;

		[[nodiscard]] BodyType getBodyType() const { return _bodyType; }

		[[nodiscard]] size_t getContentLength() const { return _contentLength; }

		// Setters
		void setMethod(const std::string &method);
		void setRequestUri(const std::string &requestUri);
		void setServerSidePath(const std::string &serverSidePath);
		void setIsFile(bool isFile);
		void setRessourceExtension(const std::string &ressourceExtension);
		void setQueryString(const std::string &queryString);
		void setLocation(const std::string &location);

		// Error 400
		class BadRequest : public std::exception {
			public:
				const char *what() const noexcept override { return "Bad request - invalid HTTP request line"; }
		};

		// Error 501
		class NotImplemented : public std::exception {
			public:
				const char *what() const noexcept override { return "Method not implemented"; }
		};

		// Error 505
		class InvalidVersion : public std::exception {
			public:
				const char *what() const noexcept override { return "HTTP version not supported"; }
		};

	private:
		std::string _method;
		std::string _requestUri;
		std::string _rawRequest;
		std::string _serverSidePath;
		bool _isFile;
		std::string _location;
		BodyType _bodyType;
		std::string _ressourceExtension;
		std::string _queryString;
		size_t _contentLength = 0;

		void parseURI();
		void validateRequestLine() const;
		void _validateRequestLine() const;
		void validateHeaders() const;
		void parseChunkedBody(std::istringstream &requestStream);
		void _initBodyType();

		static const std::vector<std::string> _supportedMethods;
		static const std::vector<std::string> _unsupportedMethods;
		static const std::vector<std::string> _supportedVersions;
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);
