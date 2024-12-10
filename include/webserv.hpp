/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:43:58 by lgreau            #+#    #+#             */
/*   Updated: 2024/12/10 19:21:33 by flfische         ###   ########.fr       */
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