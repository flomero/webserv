/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requesthandlers.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/11 14:41:46 by flfische          #+#    #+#             */
/*   Updated: 2024/10/15 10:51:24 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpStatus.hpp"
#include "Logger.hpp"

// POST request handlers
int handlePostRequest(HttpRequest &request);
int handlePostUrlEncoded(HttpRequest &request);
int handlePostMultipart(HttpRequest &request);
int handleFileUpload(const std::string &part,
					 const std::string &contentDisposition);