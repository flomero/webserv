/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMessage.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 16:30:29 by flfische          #+#    #+#             */
/*   Updated: 2024/10/05 14:03:23 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include <string>

/**
 * @brief Base class for HTTP messages - used for both requests and responses
 */
class HttpMessage {
 public:
  HttpMessage() = default;
  virtual ~HttpMessage() = default;

  // Getters
  std::string getHttpVersion() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string getBody() const;

  // Setters
  void setHttpVersion(const std::string &httpVersion);
  void setHeaders(const std::map<std::string, std::string> &headers);
  void setBody(const std::string &body);

  // Add a header to the message
  void addHeader(const std::string &key, const std::string &value);

 protected:
  std::string httpVersion = "HTTP/1.1";
  std::map<std::string, std::string> headers;
  std::string body;
};
