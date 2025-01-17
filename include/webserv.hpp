/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:43:58 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/14 10:28:23 by lgreau           ###   ########.fr       */
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
#define SIZE_BYTES_TO_SEND_BACK size_t(1024 * 1024)
