/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/05 13:49:26 by flfische          #+#    #+#             */
/*   Updated: 2024/10/28 20:42:55 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpMessage.hpp"
#include "HttpStatus.hpp"

/**
 * @brief Represents an HTTP response
 */
class HttpResponse : public HttpMessage {
	public:
		HttpResponse() = default;
		HttpResponse(Http::Status status);
		HttpResponse(int status);
		~HttpResponse() = default;
		HttpResponse(const HttpResponse &other) = default;
		HttpResponse &operator=(const HttpResponse &other) = default;

		// Setters
		void setStatus(Http::Status status);
		void setDefaultHeaders();

		// Getters
		[[nodiscard]] Http::Status getStatus() const;

		// Member Functions
		[[nodiscard]] std::string toString() const;

	private:
		Http::Status _status = Http::Status::NONE;
};
