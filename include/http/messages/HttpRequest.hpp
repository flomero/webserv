/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:15:21 by flfische          #+#    #+#             */
/*   Updated: 2024/10/06 12:39:05 by flfische         ###   ########.fr       */
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

  // Setters
  void setMethod(const std::string &method);
  void setRequestUri(const std::string &requestUri);

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

  void validate() const;
  void parseChunkedBody(std::istringstream &requestStream);

  static const std::vector<std::string> _supportedMethods;
  static const std::vector<std::string> _unsupportedMethods;
  static const std::vector<std::string> _supportedVersions;
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);