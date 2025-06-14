/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:07:36 by mgama             #+#    #+#             */
/*   Updated: 2025/06/14 10:50:07 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "readline.hpp"
#include "utils/utils.hpp"

int tty_fd = open("/dev/ttys010", O_RDWR);

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

static void
tm_rl_draw_line(const std::string &prompt, const std::vector<char>& input_buffer, int cursor_pos)
{
	std::cout << TM_RL_ER_LINE;  // Effacer la ligne actuelle
	std::cout << TM_RL_MV_CURSOR_COL("0");  // Déplacer le curseur au début de la ligne

(void)dprintf(tty_fd, "c: %zu, %d, %zu, (%s)\n", prompt.size(), cursor_pos, prompt.size() + 1 + cursor_pos, std::string(input_buffer.begin(), input_buffer.end()).c_str());

	// Afficher le prompt
	std::cout << prompt;

	// Afficher le contenu du buffer
	for (size_t i = 0; i < input_buffer.size(); i++)
	{
		std::cout << input_buffer[i];
	}

	// Déplacer le curseur à la position actuelle
	std::cout << TM_RL_ESC_SEQ TM_RL_CTRL_SEQ << prompt.size() + 1 + cursor_pos << "G";
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
		global_history_index = std::min(global_history_index + 1, global_history.size());
(void)dprintf(tty_fd, "global_history_index: %zu\n", global_history_index);
		tm_rl_draw_from_history(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ARROW_DOWN: // Down arrow
		global_history_index = global_history_index > 0 ? global_history_index - 1 : 0;
(void)dprintf(tty_fd, "global_history_index: %zu\n", global_history_index);
		if (global_history_index == 0)
		{
			input_buffer.clear();
			cursor_pos = 0;
			tm_rl_draw_line(prompt, input_buffer, cursor_pos);
			break;
		}
		tm_rl_draw_from_history(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ARROW_RIGHT: // Right arrow
		if (cursor_pos < input_buffer.size())
		{
			cursor_pos++;
			std::cout << TM_RL_MV_CURSOR_RIGHT;
		}
		break;
	case TM_RL_ARROW_LEFT: // Left arrow
		if (cursor_pos > 0)
		{
			cursor_pos--;
			std::cout << TM_RL_MV_CURSOR_LEFT;
		}
		break;
	}
}

static void
tm_rl_process_escape_ctrl_arrows(const char ch, const std::string &prompt, std::vector<char>& input_buffer, size_t& cursor_pos)
{
	(void)prompt;

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
(void)dprintf(tty_fd, "ch: %d\n", ch);

	switch (ch)
	{
	case TM_RL_CH_EOT: // Ctrl + D
		if (input_buffer.empty())
		{
			std::cout << TM_RL_EOF_SEQ << std::endl;
			return TM_RL_EOF;
		}
		break;
	case TM_RL_ESC_CTRL_A: // Ctrl + A
		cursor_pos = 0;
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ESC_CTRL_E: // Ctrl + E
		cursor_pos = input_buffer.size();
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ESC_CTRL_U: // Ctrl + U
		input_buffer.clear();
		cursor_pos = 0;
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_ESC_CTRL_W: // Ctrl + W
		tm_rl_delete_word(input_buffer, cursor_pos, false);
		tm_rl_draw_line(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_ESC: // Escape sequence
		tm_rl_process_escape_sequence(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_BS: // Backspace
	case TM_RL_CH_DEL: // Delete
		tm_rl_left_suppr(prompt, input_buffer, cursor_pos);
		break;
	case TM_RL_CH_CR:
		cursor_pos = 0;
		break;
	case TM_RL_CH_ETX:
	case TM_RL_CH_NL:
		std::cout << std::endl;
		global_history_index = 0;
		return TM_RL_NEW_LINE;
	case TM_RL_CH_HT:
		tm_rl_process_autocomplete(prompt, input_buffer, cursor_pos);
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
