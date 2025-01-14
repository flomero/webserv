/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsingErrors.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/07 17:08:16 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/14 14:21:16 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <vector>

enum eParsingErrors {
	UNEXPECTED_TOKEN,

	LISTEN_MISSING_VALUES,

	CGI_BAD_EXTENSION,
	CGI_BAD_EXECUTABLE,

	AUTOINDEX_BAD_VALUE,

	ALLOW_METHODS_MISSING_VALUES
};

#define ERROR_NAME 0
#define ERROR_TEXT 1

const std::map<eParsingErrors, std::vector<std::string> > parsingErrorsMessages = {
	{UNEXPECTED_TOKEN, {"UNEXPECTED_TOKEN", "expected: "}},

	{LISTEN_MISSING_VALUES, {"LISTEN_MISSING_VALUES", "expected: "}},

	{CGI_BAD_EXTENSION, {"CGI_BAD_EXTENSION", "expected: "}},
	{CGI_BAD_EXECUTABLE, {"CGI_BAD_EXECUTABLE", "expected: "}},

	{AUTOINDEX_BAD_VALUE, {"AUTOINDEX_BAD_VALUE", "expected: "}},

	{ALLOW_METHODS_MISSING_VALUES, {"ALLOW_METHODS_MISSING_VALUES", "expected: "}},
};
