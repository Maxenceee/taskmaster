/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:08:18 by mgama             #+#    #+#             */
/*   Updated: 2025/03/19 16:20:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READLINE_HPP
#define READLINE_HPP

#include "tm.hpp"

#define TM_RL_CH_ETX		'\x03'
#define TM_RL_CH_EOT		'\x04'
#define TM_RL_CH_BEL		'\a'
#define TM_RL_CH_BS			'\b'
#define TM_RL_CH_HT			'\t'
#define TM_RL_CH_NL			'\n'
#define TM_RL_CH_CR			'\r'
#define TM_RL_CH_ESC		'\e'
#define TM_RL_CH_SPACE		' '
#define TM_RL_CH_DEL		127

#define TM_RL_ESC_SEP		';'
#define TM_RL_ESC_DELIM		'['

#define TM_RL_MOD_EXT		'1'
#define TM_RL_MOD_SHIFT		'2'
#define TM_RL_MOD_ALT		'3'
#define TM_RL_MOD_CTRL		'5'

#define TM_RL_ESC_CTRL_A	1
#define TM_RL_ESC_CTRL_E	5
#define TM_RL_ESC_CTRL_U	21
#define TM_RL_ESC_CTRL_W	23

#define TM_RL_ARROW_UP		'A'
#define TM_RL_ARROW_DOWN	'B'
#define TM_RL_ARROW_RIGHT	'C'
#define TM_RL_ARROW_LEFT	'D'

#define TM_RL_MIN_CHAR		32
#define TM_RL_MAX_CHAR		126

#define TM_RL_ESC_SEQ		"\033"
#define TM_RL_CTRL_SEQ		"["
#define TM_RL_EOF_SEQ		"^D"

#define TM_RL_ESC(seq) TM_RL_ESC_SEQ TM_RL_CTRL_SEQ seq

#define TM_RL_MV_CURSOR_UP		TM_RL_ESC("A")
#define TM_RL_MV_CURSOR_DOWN	TM_RL_ESC("B")
#define TM_RL_MV_CURSOR_RIGHT	TM_RL_ESC("C")
#define TM_RL_MV_CURSOR_LEFT	TM_RL_ESC("D")
#define TM_RL_MV_CURSOR_COL(x)	TM_RL_ESC(x) "G"

#define TM_RL_SV_CURSOR_POS		TM_RL_ESC("s")
#define TM_RL_RS_CURSOR_POS		TM_RL_ESC("u")

#define TM_RL_ER_CURSOR_END		TM_RL_ESC("0K")
#define TM_RL_ER_CURSOR_START	TM_RL_ESC("1K")
#define TM_RL_ER_LINE			TM_RL_ESC("2K")

typedef std::vector<std::string> (*tm_rl_autocomple_handler_t)(const std::string& input, size_t cursor_pos);

std::optional<std::string>	tm_readline(const std::string& prompt);

void	tm_rl_add_history(const std::string& line);
void	tm_rl_clear_history(void);
void	tm_rl_new_line(void);
void	tm_rl_add_autocomplete_handler(tm_rl_autocomple_handler_t handler);

#endif /* READLINE_HPP */