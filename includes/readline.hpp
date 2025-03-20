/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:08:18 by mgama             #+#    #+#             */
/*   Updated: 2025/03/20 11:37:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READLINE_HPP
#define READLINE_HPP

#include "tm.hpp"
#include <sys/ioctl.h>
#include <termios.h>

#define TM_RL_CH_NUL		0
#define TM_RL_CH_SOH		1
#define TM_RL_CH_STX		2
#define TM_RL_CH_ETX		3
#define TM_RL_CH_EOT		4
#define TM_RL_CH_ENQ		5
#define TM_RL_CH_ACK		6
#define TM_RL_CH_BEL		7
#define TM_RL_CH_BS			8
#define TM_RL_CH_HT			9
#define TM_RL_CH_NL			10
#define TM_RL_CH_VT			11
#define TM_RL_CH_FF			12
#define TM_RL_CH_CR			13
#define TM_RL_CH_SO			14
#define TM_RL_CH_SI			15
#define TM_RL_CH_DLE		16
#define TM_RL_CH_DC1		17
#define TM_RL_CH_DC2		18
#define TM_RL_CH_DC3		19
#define TM_RL_CH_DC4		20
#define TM_RL_CH_NAK		21
#define TM_RL_CH_SYN		22
#define TM_RL_CH_ETB		23
#define TM_RL_CH_CAN		24
#define TM_RL_CH_EM			25
#define TM_RL_CH_SUB		26
#define TM_RL_CH_ESC		27
#define TM_RL_CH_FS			28
#define TM_RL_CH_GS			29
#define TM_RL_CH_RS			30
#define TM_RL_CH_US			31
#define TM_RL_CH_SPACE		32
#define TM_RL_CH_DEL		127

#define TM_RL_MIN_CHAR		32
#define TM_RL_MAX_CHAR		126

#define TM_RL_ESC_SEP		';'
#define TM_RL_ESC_DELIM		'['

#define TM_RL_MOD_EXT		'1'
#define TM_RL_MOD_SHIFT		'2'
#define TM_RL_MOD_ALT		'3'
#define TM_RL_MOD_CTRL		'5'

#ifdef __APPLE__
#define TM_RL_MOD_ALT_LEFT		'b'
#define TM_RL_MOD_ALT_RIGHT		'f'
#endif /* __APPLE__ */
#define TM_RL_MOD_ALT_DEL		100

#define TM_RL_ESC_CTRL_A	TM_RL_CH_SOH
#define TM_RL_ESC_CTRL_E	TM_RL_CH_ENQ
#define TM_RL_ESC_CTRL_U	TM_RL_CH_NAK
#define TM_RL_ESC_CTRL_W	TM_RL_CH_ETB

#define TM_RL_ARROW_UP		'A'
#define TM_RL_ARROW_DOWN	'B'
#define TM_RL_ARROW_RIGHT	'C'
#define TM_RL_ARROW_LEFT	'D'

#define TM_RL_ESC_SEQ		"\033"
#define TM_RL_CTRL_SEQ		"["
#define TM_RL_EOF_SEQ		"^D"

#define TM_RL_ESC(seq) TM_RL_ESC_SEQ TM_RL_CTRL_SEQ seq

#define TM_RL_MV_CURSOR_UP		TM_RL_ESC("A")
#define TM_RL_MV_CURSOR_DOWN	TM_RL_ESC("B")
#define TM_RL_MV_CURSOR_RIGHT	TM_RL_ESC("C")
#define TM_RL_MV_CURSOR_LEFT	TM_RL_ESC("D")
#define TM_RL_MV_CURSOR_COL(x)	TM_RL_ESC(x) "G"

#define TM_RL_MV_CURSOR_NEXT_LINE	TM_RL_ESC("E")
#define TM_RL_MV_CURSOR_PREV_LINE	TM_RL_ESC("F")

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