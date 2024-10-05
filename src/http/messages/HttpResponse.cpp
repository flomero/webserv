/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/05 13:53:24 by flfische          #+#    #+#             */
/*   Updated: 2024/10/05 13:59:30 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

#include <string>

#include "HttpStatus.hpp"

void HttpResponse::SetStatus(int status) { this->status = status; }

int HttpResponse::GetStatus() const { return this->status; }

std::string HttpResponse::ToString() const {
  std::string str = httpVersion + " " + std::to_string(status) + " " +
                    getStatusMessage(status) + "\r\n";
  return str;
}
