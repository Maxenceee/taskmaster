/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:08:18 by mgama             #+#    #+#             */
/*   Updated: 2025/03/20 17:56:40 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READLINE_HPP
#define READLINE_HPP

#include "tm.hpp"
#include <sys/ioctl.h>
#include <termios.h>

#define TM_RL_CH_NUL		0  // NULL
#define TM_RL_CH_SOH		1  // Moves the cursor to the beginning of the line.
#define TM_RL_CH_STX		2  // Moves the cursor one character backward.
#define TM_RL_CH_ETX		3  // Interrupts the current program (SIGINT).
#define TM_RL_CH_EOT		4  // Sends EOF if the line is empty, otherwise deletes the character under the cursor.
#define TM_RL_CH_ENQ		5  // Moves the cursor to the end of the line.
#define TM_RL_CH_ACK		6  // Moves the cursor one character forward.
#define TM_RL_CH_BEL		7  // Produces an audible beep (BELL).
#define TM_RL_CH_BS			8  // Deletes the character before the cursor.
#define TM_RL_CH_HT			9  // Inserts a tabulation.
#define TM_RL_CH_NL			10 // Executes the command.
#define TM_RL_CH_VT			11 // Cuts text from the cursor to the end of the line.
#define TM_RL_CH_FF			12 // Clears the screen.
#define TM_RL_CH_CR			13 // Executes the command.
#define TM_RL_CH_SO			14 // Displays the next command in the history.
#define TM_RL_CH_SI			15 // Executes the command and repeats the next one in the history (bash).
#define TM_RL_CH_DLE		16 // Displays the previous command in the history.
#define TM_RL_CH_DC1		17 // Resumes output after a DC3.
#define TM_RL_CH_DC2		18 // Searches for a command in the history interactively.
#define TM_RL_CH_DC3		19 // Blocks terminal output.
#define TM_RL_CH_DC4		20 // Swaps the positions of the last two characters before the cursor.
#define TM_RL_CH_NAK		21 // Cuts text from the beginning of the line to the cursor.
#define TM_RL_CH_SYN		22 // Allows inserting a literal character.
#define TM_RL_CH_ETB		23 // Cuts the previous word.
#define TM_RL_CH_CAN		24 // Various uses depending on the context.
#define TM_RL_CH_EM			25 // Pastes the last cut text.
#define TM_RL_CH_SUB		26 // Suspends the current program.
#define TM_RL_CH_ESC		27 // Used for escape sequences.
#define TM_RL_CH_FS			28 // Used to separate files in data streams.
#define TM_RL_CH_GS			29 // Logical separator for structuring data.
#define TM_RL_CH_RS			30 // Indicates separation between data records.
#define TM_RL_CH_US			31 // Even smaller separator.
#define TM_RL_CH_SPACE		32 // Space
#define TM_RL_CH_DEL		127 // Deletes the character before the cursor.

#define TM_RL_MIN_CHAR		32
#define TM_RL_MAX_CHAR		126

#define TM_RL_ESC_SEP		';'
#define TM_RL_ESC_DELIM		'['

#define TM_RL_MOD_EXT		'1' // Indicates an extended sequence
#define TM_RL_MOD_SHIFT		'2' // Indicates the Shift key is pressed
#define TM_RL_MOD_ALT		'3' // Indicates the Alt key is pressed
#define TM_RL_MOD_CTRL		'5' // Indicates the Ctrl key is pressed

#ifdef __APPLE__
#define TM_RL_MOD_ALT_LEFT		'b'
#define TM_RL_MOD_ALT_RIGHT		'f'
#endif /* __APPLE__ */
#define TM_RL_MOD_ALT_DEL		100 // Sequence for the delete key with the Alt key

#define TM_RL_ARROW_UP		'A' // Arrow key up
#define TM_RL_ARROW_DOWN	'B' // Arrow key down
#define TM_RL_ARROW_RIGHT	'C' // Arrow key right
#define TM_RL_ARROW_LEFT	'D' // Arrow key left

#define TM_RL_ESC_SEQ		"\033"
#define TM_RL_CTRL_SEQ		"["
#define TM_RL_EOF_SEQ		"^D"

#define TM_RL_ESC(seq) TM_RL_ESC_SEQ TM_RL_CTRL_SEQ seq

#define TM_RL_MV_CURSOR_UP		TM_RL_ESC("A") // Sequence to move the cursor up 
#define TM_RL_MV_CURSOR_DOWN	TM_RL_ESC("B") // Sequence to move the cursor down
#define TM_RL_MV_CURSOR_RIGHT	TM_RL_ESC("C") // Sequence to move the cursor right
#define TM_RL_MV_CURSOR_LEFT	TM_RL_ESC("D") // Sequence to move the cursor left
#define TM_RL_MV_CURSOR_COL(x)	TM_RL_ESC(x) "G" // Sequence to move the cursor to the column x

#define TM_RL_MV_CURSOR_NEXT_LINE	TM_RL_ESC("E") // Sequence to move the cursor to the next line
#define TM_RL_MV_CURSOR_PREV_LINE	TM_RL_ESC("F") // Sequence to move the cursor to the previous line

#define TM_RL_SV_CURSOR_POS		TM_RL_ESC("s") // Sequence to save the cursor position
#define TM_RL_RS_CURSOR_POS		TM_RL_ESC("u") // Sequence to restore the cursor position

#define TM_RL_ER_CURSOR_END		TM_RL_ESC("0K") // Sequence to erase the cursor to the end of the line
#define TM_RL_ER_CURSOR_START	TM_RL_ESC("1K") // Sequence to erase the cursor to the start of the line
#define TM_RL_ER_LINE			TM_RL_ESC("2K") // Sequence to erase the entire line

typedef std::vector<std::string> (*tm_rl_autocomple_handler_t)(const std::string& input, size_t cursor_pos);

std::optional<std::string>	tm_readline(const std::string& prompt);

void	tm_rl_add_history(const std::string& line);
void	tm_rl_clear_history(void);
void	tm_rl_new_line(void);
void	tm_rl_add_autocomplete_handler(tm_rl_autocomple_handler_t handler);

#endif /* READLINE_HPP */