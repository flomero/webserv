/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/05 13:49:26 by flfische          #+#    #+#             */
/*   Updated: 2024/10/22 14:55:31 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpMessage.hpp"

/**
 * @brief Represents an HTTP response
 */
class HttpResponse : public HttpMessage {
	public:
		HttpResponse() = default;
		HttpResponse(int status);
		~HttpResponse() = default;
		HttpResponse(const HttpResponse &other) = default;
		HttpResponse &operator=(const HttpResponse &other) = default;

		// Setters
		void setStatus(int status);
		void setDefaultHeaders();

		// Getters
		[[nodiscard]] int getStatus() const;

		// Member Functions
		[[nodiscard]] std::string toString() const;

	private:
		int _status = 0;
};
