/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:07:36 by mgama             #+#    #+#             */
/*   Updated: 2025/03/20 14:48:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "readline.hpp"
#include "utils/utils.hpp"

int tty_fd = open("/dev/pts/9", O_RDWR);

enum tm_rl_ev {
	TM_RL_NEW_LINE = 0,
	TM_RL_CONTINUE = 1,
	TM_RL_EOF = 2
};

std::deque<std::string>		global_history;
size_t						global_history_index = 0;
std::string					global_prompt;
size_t						global_cursor_pos = 0;
std::vector<char>			global_input_buffer;
tm_rl_autocomple_handler_t	global_autocomplete_handler = nullptr;

static char
tm_rl_getch()
{
	struct termios oldt, newt;
	char ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);  // Désactive le mode canonique et l'écho
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

static int
get_terminal_width() {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
}

static int
calculate_line_count(const std::string &prompt, const std::vector<char>& input_buffer, int term_width) {
	int total_length = prompt.size() + input_buffer.size();
	return (total_length + term_width - 1) / term_width;
}

static void
tm_rl_draw_line(const std::string &prompt, const std::vector<char>& input_buffer, int cursor_pos)
{
	int term_width = get_terminal_width();
	int line_count = calculate_line_count(prompt, input_buffer, term_width);

(void)dprintf(tty_fd, "cs %zu, tw: %d, lc: %d\n", prompt.size() + input_buffer.size(), term_width, line_count);

	for (int i = 0; i < line_count; i++)
	{
		std::cout << TM_RL_ER_LINE;
		if (i > 0)
			std::cout << TM_RL_MV_CURSOR_NEXT_LINE;
	}

	std::cout << TM_RL_MV_CURSOR_COL("0");

(void)dprintf(tty_fd, "c: %zu, %d, %zu, (%s)\n", prompt.size(), cursor_pos, prompt.size() + 1 + cursor_pos, std::string(input_buffer.begin(), input_buffer.end()).c_str());

	// Afficher le prompt
	std::cout << prompt;

	// Afficher le contenu du buffer
	for (size_t i = 0; i < input_buffer.size(); i++)
	{
		std::cout << input_buffer[i];
	}

	if ((prompt.size() + input_buffer.size()) % term_width == 0) {
		std::cout << "\n";  // Ajoute une ligne fantôme (comme le fait Zsh)
	}

	size_t total_rows = (prompt.size() + input_buffer.size()) / term_width;
	size_t current_row = (prompt.size() + cursor_pos) / term_width;
	size_t row_offset = total_rows - current_row;

	// Déplacer le curseur à la bonne ligne (remonter vers la ligne du prompt)
	if (row_offset > 0) {
		std::cout << "\033[" << row_offset << "A"; // Remonte de `row_offset` lignes
	}

	// Positionner le curseur sur la bonne colonne
	size_t cursor_col = (prompt.size() + cursor_pos) % term_width;

(void)dprintf(tty_fd, "ttr: %zu, cr: %zu, ro: %zu, co: %zu", total_rows, current_row, row_offset, cursor_col);

	std::cout << TM_RL_ESC_SEQ TM_RL_CTRL_SEQ << cursor_col + 1 << "G";
	// std::cout << TM_RL_ESC_SEQ TM_RL_CTRL_SEQ << prompt.size() + 1 + cursor_pos << "G";
}

static void
tm_rl_draw_from_history(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	// History start at 1 because 0 is the current input
	if (global_history_index > 0 && global_history_index < global_history.size() + 1)
	{
		input_buffer.assign(global_history[global_history_index - 1].begin(), global_history[global_history_index - 1].end());
		cursor_pos = input_buffer.size();
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
	}
}

static void
tm_rl_draw_previous_history(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	global_history_index = std::min(global_history_index + 1, global_history.size());
(void)dprintf(tty_fd, "global_history_index: %zu\n", global_history_index);
	tm_rl_draw_from_history(prompt, input_buffer, cursor_pos);
}

static void
tm_rl_draw_next_history(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	global_history_index = global_history_index > 0 ? global_history_index - 1 : 0;
(void)dprintf(tty_fd, "global_history_index: %zu\n", global_history_index);
	if (global_history_index == 0)
	{
		input_buffer.clear();
		cursor_pos = 0;
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		return;
	}
	tm_rl_draw_from_history(prompt, input_buffer, cursor_pos);
}

static void
tm_rl_process_autocomplete(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	if (!global_autocomplete_handler)
		return;

	std::string input(input_buffer.begin(), input_buffer.end());
(void)dprintf(tty_fd, "input_buffer: %s\n", input.c_str());

	// Récupérer les suggestions d'autocomplétion
	std::vector<std::string> suggestions = global_autocomplete_handler(input, cursor_pos);

	if (suggestions.empty())
		return;

	if (suggestions.size() == 1)
	{
		size_t start_of_token = input.rfind(TM_RL_CH_SPACE, cursor_pos > 0 ? cursor_pos - 1 : 0);
		start_of_token = (start_of_token == std::string::npos) ? 0 : start_of_token + 1;

		size_t end_of_token = input.find(TM_RL_CH_SPACE, cursor_pos);
		end_of_token = (end_of_token == std::string::npos) ? input.size() : end_of_token;

		// Extraire les parties avant, après et le token actif
		std::string prefix = input.substr(0, start_of_token);
		std::string suffix = input.substr(end_of_token);
		std::string active_token = input.substr(start_of_token, end_of_token - start_of_token);

		// Construire la ligne complétée
		const std::string& suggestion = suggestions[0];
		std::string completed = prefix + suggestion;
		if (cursor_pos == end_of_token && (end_of_token >= input.size() || input[end_of_token] != TM_RL_CH_SPACE)) // Ajouter un espace si le curseur est à la fin du token
		{
			completed += TM_RL_CH_SPACE;
		}
		completed += suffix;

		// Mettre à jour le buffer d'entrée et le curseur
		input_buffer = std::vector<char>(completed.begin(), completed.end());
		cursor_pos = prefix.size() + suggestion.size() + (cursor_pos == end_of_token ? 1 : 0);

		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
	}
	else
	{
		// Afficher les suggestions
		std::cout << TM_RL_CH_NL;
		for (const auto& suggestion : suggestions)
		{
			std::cout << suggestion << TM_RL_CH_SPACE;
		}
		std::cout << TM_RL_CH_NL;

		// Réafficher la ligne actuelle
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
	}
}

static void
tm_rl_left_suppr(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos, int count = 1)
{
	if (cursor_pos > 0)
	{
		input_buffer.erase(input_buffer.begin() + cursor_pos - count, input_buffer.begin() + cursor_pos);

		cursor_pos -= count;
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
	}
}

static void
tm_rl_right_suppr(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos, int count = 1)
{
	if (cursor_pos < input_buffer.size())
	{
		input_buffer.erase(input_buffer.begin() + cursor_pos, input_buffer.begin() + cursor_pos + count);

		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
	}
}

static void
tm_rl_add_char(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos, char ch)
{
	if (ch < TM_RL_MIN_CHAR || ch > TM_RL_MAX_CHAR)
	{
		return;
	}
	input_buffer.insert(input_buffer.begin() + cursor_pos, ch);
	cursor_pos++;
	tm_rl_draw_line(prompt, input_buffer, cursor_pos);
}

static void
tm_rl_move_cursor_left(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	if (cursor_pos > 0)
	{
		cursor_pos--;
		std::cout << TM_RL_MV_CURSOR_LEFT;
	}
}

static void
tm_rl_move_cursor_right(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	if (cursor_pos < input_buffer.size())
	{
		cursor_pos++;
		std::cout << TM_RL_MV_CURSOR_RIGHT;
	}
}

static void
tm_rl_move_cursor_by_word(const std::vector<char>& input_buffer, size_t& cursor_pos, bool forward)
{
	if (forward)
	{
		// Skip any spaces first
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] == TM_RL_CH_SPACE)
		{
			cursor_pos++;
			std::cout << TM_RL_MV_CURSOR_RIGHT;
		}
		// Then move to the end of the word
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] != TM_RL_CH_SPACE)
		{
			cursor_pos++;
			std::cout << TM_RL_MV_CURSOR_RIGHT;
		}
	}
	else 
	{
		// Skip any spaces first
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] == TM_RL_CH_SPACE)
		{
			cursor_pos--;
			std::cout << TM_RL_MV_CURSOR_LEFT;
		}
		// Then move to the beginning of the word
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] != TM_RL_CH_SPACE)
		{
			cursor_pos--;
			std::cout << TM_RL_MV_CURSOR_LEFT;
		}
	}
}

static void
tm_rl_delete_word(std::vector<char>& input_buffer, size_t& cursor_pos, bool forward)
{
	if (forward) 
	{
		// Skip any spaces first
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] == TM_RL_CH_SPACE)
		{
			input_buffer.erase(input_buffer.begin() + cursor_pos);
		}
		// Then delete the word
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] != TM_RL_CH_SPACE)
		{
			input_buffer.erase(input_buffer.begin() + cursor_pos);
		}
	}
	else
	{
		// Skip any spaces first
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] == TM_RL_CH_SPACE)
		{
			input_buffer.erase(input_buffer.begin() + cursor_pos - 1);
			cursor_pos--;
		}
		// Then delete the word
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] != TM_RL_CH_SPACE)
		{
			input_buffer.erase(input_buffer.begin() + cursor_pos - 1);
			cursor_pos--;
		}
	}
}

static void
tm_rl_process_escape_basic_arrows(const char ch, const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	switch (ch)
	{
	case TM_RL_ARROW_UP: // Up arrow
		tm_rl_draw_previous_history(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ARROW_DOWN: // Down arrow
		tm_rl_draw_next_history(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ARROW_RIGHT: // Right arrow
		tm_rl_move_cursor_right(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ARROW_LEFT: // Left arrow
		tm_rl_move_cursor_left(prompt, input_buffer, cursor_pos);
		break;
	}
}

static void
tm_rl_process_escape_ctrl_arrows(const char ch, const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	switch (ch)
	{
	case TM_RL_ARROW_RIGHT: // Right arrow
		tm_rl_move_cursor_by_word(input_buffer, cursor_pos, true);
		break;
	case TM_RL_ARROW_LEFT: // Left arrow
		tm_rl_move_cursor_by_word(input_buffer, cursor_pos, false);
		break;
	}
}

static void
tm_rl_process_modified_arrow(const std::string& prompt, std::vector<char>& input_buffer, size_t& cursor_pos, int modifier, char direction)
{
(void)dprintf(tty_fd, "mod: %c, dir: %c\n", modifier, direction);
	switch (modifier)
	{
	case TM_RL_MOD_CTRL:
	case TM_RL_MOD_ALT:
		tm_rl_process_escape_ctrl_arrows(direction, prompt, input_buffer, cursor_pos);
		break;
	default: // Unhandled modifier
		tm_rl_add_char(prompt, input_buffer, cursor_pos, direction);
	}
}

static void
tm_rl_process_escape_sequence(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	char ch = tm_rl_getch();

(void)dprintf(tty_fd, "es ch: %c\n", ch);

	switch (ch)
	{
#ifdef __APPLE__
	case TM_RL_MOD_ALT_LEFT: // Alt + Left arrow
		tm_rl_move_cursor_by_word(input_buffer, cursor_pos, false);
		return;
	case TM_RL_MOD_ALT_RIGHT: // Alt + Right arrow
		tm_rl_move_cursor_by_word(input_buffer, cursor_pos, true);
		return;
#endif /* __APPLE__ */
	case TM_RL_MOD_ALT_DEL: // Alt + Right Suppr
		tm_rl_delete_word(input_buffer, cursor_pos, true);
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		return;
	case TM_RL_ESC_DELIM:
		// If escape sequence break out of the statement
		break;
	default:
		return; // Invalid escape sequence
	}

	ch = tm_rl_getch();
	
(void)dprintf(tty_fd, "es m ch: %c\n", ch);

	// Check if the character is a modifier
	switch (ch)
	{
	case TM_RL_MOD_EXT: // Extended arrow sequence
		if (tm_rl_getch() == TM_RL_ESC_SEP)
		{
			char mod = tm_rl_getch();
			char direction = tm_rl_getch();
			tm_rl_process_modified_arrow(prompt, input_buffer, cursor_pos, mod, direction);
		}
		break;
	case TM_RL_MOD_ALT: // Alt/Cmd + Right Suppr
		if (tm_rl_getch() == TM_RL_ESC_SEP)
		{
			tm_rl_getch(); // Get modifier
			tm_rl_getch(); // Get ~
			input_buffer.clear();
			cursor_pos = 0;
			tm_rl_draw_line(prompt, input_buffer, cursor_pos);
			break;
		}
		tm_rl_right_suppr(prompt, input_buffer, cursor_pos);
		break;
	default:
		tm_rl_process_escape_basic_arrows(ch, prompt, input_buffer, cursor_pos);
		break;
	}
}

static int
tm_rl_process_input(const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	char ch = tm_rl_getch();
(void)dprintf(tty_fd, "=====\nch: %d\n", ch);

	switch (ch)
	{
	case TM_RL_CH_SOH: // Ctrl + A
		cursor_pos = 0;
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_STX: // Ctrl + B
		tm_rl_move_cursor_left(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_EOT: // Ctrl + D
		if (input_buffer.empty())
		{
			std::cout << TM_RL_EOF_SEQ << std::endl;
			return TM_RL_EOF;
		}
		else
		{
			tm_rl_right_suppr(prompt, input_buffer, cursor_pos);
		}
		break;
	case TM_RL_CH_ENQ: // Ctrl + E
		cursor_pos = input_buffer.size();
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_ACK: // Ctrl + F
		tm_rl_move_cursor_right(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_BEL: // Ctrl + G
		tm_rl_new_line();
		break;
	case TM_RL_CH_BS: // Backspace (Ctrl + H)
	case TM_RL_CH_DEL: // Delete
		tm_rl_left_suppr(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_HT: // Ctrl + I
		tm_rl_process_autocomplete(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_ETX: // Ctrl + C (only used when SIGINT is ignored)
	case TM_RL_CH_NL: // Ctrl + J or Ctrl + M
	case TM_RL_CH_SI: // Ctrl + O
		std::cout << std::endl;
		global_history_index = 0;
		return TM_RL_NEW_LINE;
	case TM_RL_CH_VT: // Ctrl + K
		input_buffer.erase(input_buffer.begin() + cursor_pos, input_buffer.end());
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_CR: // Ctrl + M
		cursor_pos = 0;
		break;
	case TM_RL_CH_SO: // Ctrl + N
		tm_rl_draw_next_history(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_DLE: // Ctrl + P
		tm_rl_draw_previous_history(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_NAK: // Ctrl + U
		input_buffer.clear();
		cursor_pos = 0;
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_ETB: // Ctrl + W
		tm_rl_delete_word(input_buffer, cursor_pos, false);
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_FF: // Ctrl + L
	case TM_RL_CH_DC1: // Ctrl + Q
	case TM_RL_CH_DC2: // Ctrl + R
	case TM_RL_CH_DC3: // Ctrl + S
	case TM_RL_CH_DC4: // Ctrl + T
	case TM_RL_CH_SYN: // Ctrl + V
	case TM_RL_CH_CAN: // Ctrl + X
	case TM_RL_CH_EM: // Ctrl + Y
	case TM_RL_CH_SUB: // Ctrl + Z
		break;
	case TM_RL_CH_ESC: // Escape sequence
		tm_rl_process_escape_sequence(prompt, input_buffer, cursor_pos);
		break;
	default:
		tm_rl_add_char(prompt, input_buffer, cursor_pos, ch);
		break;
	}
	return TM_RL_CONTINUE;
}

/**
 * @brief Read a line from the terminal.
 * 
 * @param prompt A string to display as the prompt
 * @return An optional string that should be checked before use (std::nullopt if EOF)
 */
std::optional<std::string>
tm_readline(const std::string& prompt)
{
	int status;

	global_prompt = prompt;
	global_input_buffer.clear();
	global_cursor_pos = 0;

	tm_rl_draw_line(global_prompt, global_input_buffer, global_cursor_pos);

	while ((status = tm_rl_process_input(global_prompt, global_input_buffer, global_cursor_pos)) != TM_RL_NEW_LINE)
	{
		if (status == TM_RL_EOF)
		{
			return std::nullopt;
		}
	}

	return std::string(global_input_buffer.begin(), global_input_buffer.end());
}

/**
 * @brief Add a line to the history.
 * 
 * @param line The line to add
 */
void
tm_rl_add_history(const std::string& line)
{
	if (line.empty() || is_spaces(line))
	{
		return;
	}

	if (global_history.size() > 0 && global_history.front() == line)
	{
		return;
	}

	global_history.push_front(line);
(void)dprintf(tty_fd, "global_history: %zu\n", global_history.size());
	global_history_index = 0;
}

/**
 * @brief Clear the history.
 * 
 */
void
tm_rl_clear_history(void)
{
	global_history.clear();
	global_history_index = 0;
}

/**
 * @brief Display a new entry line.
 * 
 */
void
tm_rl_new_line()
{
	// Print a newline character to move to the next line
	std::cout << std::endl;

	// Clear the input buffer and reset the cursor position
	global_input_buffer.clear();
	global_cursor_pos = 0;

	// Draw the new prompt line
	tm_rl_draw_line(global_prompt, global_input_buffer, global_cursor_pos);
}

/**
 * @brief Add an autocomplete handler.
 * 
 * If a handler is already set, it will be replaced.
 * 
 * @param handler The handler to add
 */
void
tm_rl_add_autocomplete_handler(tm_rl_autocomple_handler_t handler)
{
(void)dprintf(tty_fd, "add handler %p\n", handler);
	global_autocomplete_handler = handler;
}