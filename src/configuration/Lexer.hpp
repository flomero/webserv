/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 13:20:10 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/02 13:20:52 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <string>
# include <vector>

enum TokenType {
	TOKEN_SERVER,
	TOKEN_LOCATION,
	TOKEN_PORT,
	TOKEN_HOST,
	TOKEN_SERVER_NAME,
	TOKEN_ERROR_PAGE,
	TOKEN_CLIENT_BODY_SIZE,
	TOKEN_METHODS,
	TOKEN_ROOT,
	TOKEN_INDEX,
	TOKEN_AUTOINDEX,
	TOKEN_UPLOAD_DIR,
	TOKEN_CGI,
	TOKEN_REDIRECT,
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_ON,
	TOKEN_OFF,
	TOKEN_OPEN_BRACE,
	TOKEN_CLOSE_BRACE,
	TOKEN_SEMICOLON,
	TOKEN_EOF,
	TOKEN_INVALID
};
