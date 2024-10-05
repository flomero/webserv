/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:15:21 by flfische          #+#    #+#             */
/*   Updated: 2024/10/05 14:03:34 by flfische         ###   ########.fr       */
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

  class InvalidRequest : public std::exception {
   public:
    const char *what() const noexcept override {
      return "Invalid HTTP request";
    }
  };

  class InvalidMethod : public std::exception {
   public:
    const char *what() const noexcept override { return "Method not allowed"; }
  };

  class InvalidVersion : public std::exception {
   public:
    const char *what() const noexcept override {
      return "HTTP version not supported";
    }
  };

 private:
  HttpRequest() = default;
  std::string method;
  std::string requestUri;

  void validate() const;
  static const std::vector<std::string> supportedMethods;
  static const std::vector<std::string> supportedVersions;
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &request);