/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsingErrors.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/07 17:08:16 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/07 17:36:29 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <map>
# include <string>

enum eParsingErrors {
	UNEXPECTED_TOKEN
};

const std::map<eParsingErrors, std::string> parsingErrorsMessages = {
	{UNEXPECTED_TOKEN, "expected: "}
};
