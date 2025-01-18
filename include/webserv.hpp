/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:43:58 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/18 16:01:23 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpRequest.hpp"
#include "HttpStatus.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include "RequestHandler.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"

#define SERVER_NAME "webserv"

#define DEFAULT_POLL_TIMEOUT 5000
#define DEFAULT_CGI_TIMEOUT_MS 100000

#define SIZE_BYTES_TO_SEND_BACK size_t(1024 * 1024)
#define GET_READ_SIZE size_t(1024 * 1024)
#define POST_WRITE_SIZE size_t(1024 * 1024)
#define CGI_READ_BUFFER_SIZE size_t(1024 * 1024)
