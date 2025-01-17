/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsingErrors.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/07 17:08:16 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/17 15:11:45 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <vector>

enum eParsingErrors {
	UNEXPECTED_TOKEN,
	INVALID_UNIT,

	LISTEN_MISSING_VALUES,

	CGI_BAD_EXTENSION,
	CGI_BAD_EXECUTABLE,

	AUTOINDEX_BAD_VALUE,

	ALLOW_METHODS_MISSING_VALUES,

	SERVER_NAME_MISSING_VALUES
};

#define ERROR_NAME 0
#define ERROR_TEXT 1

#define POSSIBLE_SERVER_CONFIGS                                                                                 \
	"'location', 'listen', 'server_name', 'root', 'index', 'client_max_body_size', 'client_body_buffer_size', " \
	"'client_header_buffer_size', 'uplaod_dir', 'request_timeout' or 'error_page'"
#define POSSIBLE_ROUTE_CONFIGS                                                                                        \
	"'root', 'index', 'client_max_body_size', 'client_body_buffer_size', 'client_header_buffer_size', 'uplaod_dir', " \
	"'allow_methods', 'autoindex', 'alias', 'cgi' or 'return'"

const std::map<eParsingErrors, std::vector<std::string> > parsingErrorsMessages = {
	{UNEXPECTED_TOKEN, {"UNEXPECTED_TOKEN", "expected: "}},
	{INVALID_UNIT, {"INVALID_UNIT", "expected: "}},

	{LISTEN_MISSING_VALUES, {"LISTEN_MISSING_VALUES", "expected: "}},

	{CGI_BAD_EXTENSION, {"CGI_BAD_EXTENSION", "expected: "}},
	{CGI_BAD_EXECUTABLE, {"CGI_BAD_EXECUTABLE", "expected: "}},

	{AUTOINDEX_BAD_VALUE, {"AUTOINDEX_BAD_VALUE", "expected: "}},

	{ALLOW_METHODS_MISSING_VALUES, {"ALLOW_METHODS_MISSING_VALUES", "expected: "}},

	{SERVER_NAME_MISSING_VALUES, {"SERVER_NAME_MISSING_VALUES", "expected: "}},
};
