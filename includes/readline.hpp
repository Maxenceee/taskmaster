/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:08:18 by mgama             #+#    #+#             */
/*   Updated: 2025/01/21 18:18:51 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READLINE_HPP
#define READLINE_HPP

#include "tm.hpp"

typedef std::vector<std::string> (*tm_rl_autocomple_handler_t)(const std::string& input, size_t cursor_pos);

std::optional<std::string>	tm_readline(const std::string& prompt);

void	tm_rl_add_history(const std::string& line);
void	tm_rl_clear_history(void);
void	tm_rl_new_line(void);
void	tm_rl_add_autocomplete_handler(tm_rl_autocomple_handler_t handler);

#endif /* READLINE_HPP */