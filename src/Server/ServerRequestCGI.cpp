/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerRequestCGI.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 16:54:21 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/15 16:56:21 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Logger.hpp"

void Server::handleRequestCGI(HttpRequest& request, Route& route) {
	LOG_INFO("Entered handleRequestCGI");
	(void)request;
	(void)route;
}
