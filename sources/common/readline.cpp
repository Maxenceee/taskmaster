/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:07:36 by mgama             #+#    #+#             */
/*   Updated: 2025/01/21 16:44:49 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "readline.hpp"
#include <termios.h>
#include "utils/utils.hpp"

int tty_fd = open("/dev/ttys007", O_RDWR);

enum tm_rl_ev {
	TM_RL_NEW_LINE = 0,
	TM_RL_CONTINUE = 1,
	TM_RL_EOF = 2
};

std::deque<std::string>		global_history;
size_t						global_history_index = 0;
std::string					global_prompt;
int							global_cursor_pos = 0;
std::vector<char>			global_input_buffer;

char
getch()
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

void
draw_line(const std::string &prompt, const std::vector<char>& input_buffer, int cursor_pos)
{
	std::cout << "\033[2K";  // Effacer la ligne actuelle
	std::cout << "\033[0G";  // Déplacer le curseur au début de la ligne

	dprintf(tty_fd, "c: %zu, %d, %zu, (%s)\n", prompt.size(), cursor_pos, prompt.size() + 1 + cursor_pos, std::string(input_buffer.begin(), input_buffer.end()).c_str());

	// Afficher le prompt
	std::cout << prompt;

	// Afficher le contenu du buffer
	for (size_t i = 0; i < input_buffer.size(); i++) {
		std::cout << input_buffer[i];
	}

	// Déplacer le curseur à la position actuelle
	std::cout << "\033[" << prompt.size() + 1 + cursor_pos << "G";
}

void
draw_from_history(const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos)
{
	// History start at 1 because 0 is the current input
	if (global_history_index > 0 && global_history_index < global_history.size() + 1)
	{
		input_buffer.assign(global_history[global_history_index - 1].begin(), global_history[global_history_index - 1].end());
		cursor_pos = input_buffer.size();
		draw_line(prompt, input_buffer, cursor_pos);
	}
}

void
left_suppr(const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos, int count = 1)
{
	if (cursor_pos > 0) {
		input_buffer.erase(input_buffer.begin() + cursor_pos - count, input_buffer.begin() + cursor_pos);

		cursor_pos -= count;
		draw_line(prompt, input_buffer, cursor_pos);
	}
}

void
right_suppr(const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos, int count = 1)
{
	if (cursor_pos < input_buffer.size()) {
		input_buffer.erase(input_buffer.begin() + cursor_pos, input_buffer.begin() + cursor_pos + count);

		draw_line(prompt, input_buffer, cursor_pos);
	}
}

void
add_char(const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos, char ch)
{
	if (ch < 32 || ch > 126) {
		return;
	}
	input_buffer.insert(input_buffer.begin() + cursor_pos, ch);
	cursor_pos++;
	draw_line(prompt, input_buffer, cursor_pos);
}

void
move_cursor_by_word(const std::vector<char>& input_buffer, int& cursor_pos, bool forward)
{
	if (forward) {
		// Skip any spaces first
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] == ' ') {
			cursor_pos++;
			std::cout << "\033[C";
		}
		// Then move to the end of the word
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] != ' ') {
			cursor_pos++;
			std::cout << "\033[C";
		}
	} else {
		// Skip any spaces first
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] == ' ') {
			cursor_pos--;
			std::cout << "\033[D";
		}
		// Then move to the beginning of the word
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] != ' ') {
			cursor_pos--;
			std::cout << "\033[D";
		}
	}
}

void
delete_word(std::vector<char>& input_buffer, int& cursor_pos, bool forward)
{
	if (forward) {
		// Skip any spaces first
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] == ' ') {
			input_buffer.erase(input_buffer.begin() + cursor_pos);
		}
		// Then delete the word
		while (cursor_pos < input_buffer.size() && input_buffer[cursor_pos] != ' ') {
			input_buffer.erase(input_buffer.begin() + cursor_pos);
		}
	} else {
		// Skip any spaces first
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] == ' ') {
			input_buffer.erase(input_buffer.begin() + cursor_pos - 1);
			cursor_pos--;
		}
		// Then delete the word
		while (cursor_pos > 0 && input_buffer[cursor_pos - 1] != ' ') {
			input_buffer.erase(input_buffer.begin() + cursor_pos - 1);
			cursor_pos--;
		}
	}
}

void
process_escape_basic_arrows(const char ch, const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos)
{
	switch (ch)
	{
	case 'A': // Up arrow
		global_history_index = std::min(global_history_index + 1, global_history.size());
		dprintf(tty_fd, "global_history_index: %zu\n", global_history_index);
		draw_from_history(prompt, input_buffer, cursor_pos);
		break;
	case 'B': // Down arrow
		global_history_index = global_history_index > 0 ? global_history_index - 1 : 0;
		dprintf(tty_fd, "global_history_index: %zu\n", global_history_index);
		if (global_history_index == 0) {
			input_buffer.clear();
			cursor_pos = 0;
			draw_line(prompt, input_buffer, cursor_pos);
			break;
		}
		draw_from_history(prompt, input_buffer, cursor_pos);
		break;
	case 'C': // Right arrow
		if (cursor_pos < input_buffer.size()) {
			cursor_pos++;
			std::cout << "\033[C";  // Déplacer le curseur à droite
		}
		break;
	case 'D': // Left arrow
		if (cursor_pos > 0) {
			cursor_pos--;
			std::cout << "\033[D";  // Déplacer le curseur à gauche
		}
		break;
	}
}

void
process_escape_ctrl_arrows(const char ch, const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos)
{
	switch (ch)
	{
	case 'C': // Right arrow
		move_cursor_by_word(input_buffer, cursor_pos, true);
		break;
	case 'D': // Left arrow
		move_cursor_by_word(input_buffer, cursor_pos, false);
		break;
	}
}

void
process_modified_arrow(const std::string& prompt, std::vector<char>& input_buffer, int& cursor_pos, int modifier, char direction)
{
	switch (modifier)
	{
	case '5': // CTRL
	case '3': // ALT
		process_escape_ctrl_arrows(direction, prompt, input_buffer, cursor_pos);
		break;
	default: // Unhandled modifier
		add_char(prompt, input_buffer, cursor_pos, direction);
	}
}

void
process_escape_sequence(const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos)
{
	char ch = getch();
	dprintf(tty_fd, "es ch: %c\n", ch);
	switch (ch)
	{
#ifdef __APPLE__
	case 'b': // Alt + Left arrow
		move_cursor_by_word(input_buffer, cursor_pos, false);
		return;
	case 'f': // Alt + Right arrow
		move_cursor_by_word(input_buffer, cursor_pos, true);
		return;
#endif /* __APPLE__ */
	case 100: // Alt + Right Suppr
		delete_word(input_buffer, cursor_pos, true);
		draw_line(prompt, input_buffer, cursor_pos);
		return;
	case '[':
		// If escape sequence break out of the statement
		break;
	default:
		return; // Invalid escape sequence
	}

	ch = getch();
	dprintf(tty_fd, "es m ch: %c\n", ch);

	// Check if the character is a modifier
	switch (ch)
	{
	case '1': // Extended arrow sequence
		if (getch() == ';') {
			char mod = getch();
			char direction = getch();
			process_modified_arrow(prompt, input_buffer, cursor_pos, mod, direction);
		}
		break;
	case '3': // Right Suppr
		getch();
		right_suppr(prompt, input_buffer, cursor_pos);
		break;
	default:
		process_escape_basic_arrows(ch, prompt, input_buffer, cursor_pos);
		break;
	}
}

int
process_input(const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos)
{
	char ch = getch();
	dprintf(tty_fd, "ch: %d\n", ch);

	switch (ch)
	{
	case 4: // Ctrl + D
		if (input_buffer.empty()) {
			std::cout << "^D" << std::endl;
		    return TM_RL_EOF;
		}
		break;
	case 21: // Ctrl + U
		input_buffer.clear();
		cursor_pos = 0;
		draw_line(prompt, input_buffer, cursor_pos);
		break;
	case 23: // Ctrl + W
		delete_word(input_buffer, cursor_pos, false);
		draw_line(prompt, input_buffer, cursor_pos);
		break;
	case 27: // Escape sequence
		process_escape_sequence(prompt, input_buffer, cursor_pos);
		break;
	case 8: // Backspace
	case 127: // Delete
		left_suppr(prompt, input_buffer, cursor_pos);
		break;
	case '\r':
		cursor_pos = 0;
		break;
	case 3:
	case '\n':
		std::cout << std::endl;
		global_history_index = 0;
		return TM_RL_NEW_LINE;
	case '\t':
		// TODO: Implement tab completion
		break;
	default:
		add_char(prompt, input_buffer, cursor_pos, ch);
		break;
	}
	return TM_RL_CONTINUE;
}

/**
 * @brief Read a line from the terminal
 * 
 * @param prompt A string to display as the prompt
 * @return An optional string that should be checked before use (std::nullopt if EOF)
 */
std::optional<std::string>
tm_readline(const std::string& prompt)
{
	global_prompt = prompt;
    global_input_buffer.clear();
    global_cursor_pos = 0;

	draw_line(global_prompt, global_input_buffer, global_cursor_pos);

	int status;
	while ((status = process_input(global_prompt, global_input_buffer, global_cursor_pos)) != TM_RL_NEW_LINE) {
		if (status == TM_RL_EOF) {
			return std::nullopt;
		}
	}

	return std::string(global_input_buffer.begin(), global_input_buffer.end());
}

/**
 * @brief Add a line to the history
 * 
 * @param line The line to add
 */
void
tm_rl_add_history(const std::string& line)
{
	if (line.empty() || is_spaces(line)) {
		return;
	}
	if (global_history.size() > 0 && global_history.front() == line) {
		return;
	}
	global_history.push_front(line);
	dprintf(tty_fd, "global_history: %zu\n", global_history.size());
	global_history_index = 0;
}

/**
 * @brief Clear the history
 * 
 */
void
tm_rl_clear_history(void)
{
	global_history.clear();
	global_history_index = 0;
}

/**
 * @brief Display a new entry line
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
	draw_line(global_prompt, global_input_buffer, global_cursor_pos);
}