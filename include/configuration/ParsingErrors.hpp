/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsingErrors.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/07 17:08:16 by lgreau            #+#    #+#             */
/*   Updated: 2025/01/14 12:29:42 by lgreau           ###   ########.fr       */
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
};

#define ERROR_NAME 0
#define ERROR_TEXT 1

const std::map<eParsingErrors, std::vector<std::string> > parsingErrorsMessages = {
	{UNEXPECTED_TOKEN, {"UNEXPECTED_TOKEN", "expected: "}},

	{LISTEN_MISSING_VALUES, {"LISTEN_MISSING_VALUES", "expected: "}},
	{CGI_BAD_EXTENSION, {"CGI_BAD_EXTENSION", "expected: "}},
	{CGI_BAD_EXECUTABLE, {"CGI_BAD_EXECUTABLE", "expected: "}},
};
