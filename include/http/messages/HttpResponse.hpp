/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/05 13:49:26 by flfische          #+#    #+#             */
/*   Updated: 2024/10/09 15:11:51 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpMessage.hpp"

/**
 * @brief Represents an HTTP response
 */
class HttpResponse : public HttpMessage {
	public:
		HttpResponse(int status);
		~HttpResponse() = default;
		HttpResponse(const HttpResponse &other) = default;
		HttpResponse &operator=(const HttpResponse &other) = default;

		// Setters
		void SetStatus(int status);
		void setDefaultHeaders();

		// Getters
		[[nodiscard]] int GetStatus() const;

		// Member Functions
		[[nodiscard]] std::string ToString() const;

	private:
		HttpResponse() = default;
		int _status = 0;
};
