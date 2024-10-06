/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_iomanip.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgreau <lgreau@student.42heilbronn.de>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 14:28:15 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/06 14:29:59 by lgreau           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iomanip>

#define RESET_COLOR   "\033[0m"
#define BLACK         "\033[30m"
#define RED           "\033[31m"
#define GREEN         "\033[32m"
#define YELLOW        "\033[33m"
#define BLUE          "\033[34m"
#define MAGENTA       "\033[35m"
#define CYAN          "\033[36m"
#define WHITE         "\033[37m"

// Macro to simplify coloring text in the output
#define COLOR(color, text) (std::string(color) + text + RESET_COLOR)
