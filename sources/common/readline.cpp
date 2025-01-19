/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:07:36 by mgama             #+#    #+#             */
/*   Updated: 2025/01/19 23:06:30 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "readline.hpp"
#include <termios.h>

int tty_fd = open("/dev/ttys007", O_RDWR);

std::deque<std::string>		history;
size_t						history_index = 0;
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
	if (history_index > 0 && history_index < history.size() + 1)
	{
		input_buffer.assign(history[history_index - 1].begin(), history[history_index - 1].end());
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
escape_sequence(const std::string &prompt, std::vector<char>& input_buffer, int& cursor_pos)
{
	switch (getch())
	{
	case 'b': // Alt + Left arrow
		move_cursor_by_word(input_buffer, cursor_pos, false);
		return;
	case 'f': // Alt + Right arrow
		move_cursor_by_word(input_buffer, cursor_pos, true);
		return;
	case 100: // Alt + Suppr
		delete_word(input_buffer, cursor_pos, true);
		draw_line(prompt, input_buffer, cursor_pos);
		return;
	case '[':
		break;
	default:
		return; // Invalid escape sequence
	}

	switch (getch())
	{
	case 'A': // Up arrow
		history_index = std::min(history_index + 1, history.size());
		dprintf(tty_fd, "history_index: %zu\n", history_index);
		draw_from_history(prompt, input_buffer, cursor_pos);
		break;
	case 'B': // Down arrow
		history_index = history_index > 0 ? history_index - 1 : 0;
		dprintf(tty_fd, "history_index: %zu\n", history_index);
		if (history_index == 0) {
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
	case 51: // Suppr
		switch (getch())
		{
		case '~':
			right_suppr(prompt, input_buffer, cursor_pos);
			break;
		}
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
		    return 2;
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
		escape_sequence(prompt, input_buffer, cursor_pos);
		break;
	case 8: // Backspace
	case 127: // Suppr
		left_suppr(prompt, input_buffer, cursor_pos);
		break;
	case '\r':
		cursor_pos = 0;
		break;
	case 3:
	case '\n':
		std::cout << std::endl;
		history_index = 0;
		return 0;
	case '\t':
		// TODO: Implement tab completion
		break;
	default:
		add_char(prompt, input_buffer, cursor_pos, ch);
		break;
	}
	return 1;
}

/**
 * @brief Read a line from the terminal
 * 
 * @param prompt A string to display as the prompt
 * @return std::optional<std::string> 
 */
std::optional<std::string>
tm_readline(const std::string& prompt)
{
	global_prompt = prompt;
    global_input_buffer.clear();
    global_cursor_pos = 0;

	draw_line(global_prompt, global_input_buffer, global_cursor_pos);

	int status;
	while ((status = process_input(global_prompt, global_input_buffer, global_cursor_pos)) != 0) {
		if (status == 2) {
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
	if (line.empty()) {
		return;
	}
	if (history.size() > 0 && history.front() == line) {
		return;
	}
	history.push_front(line);
	dprintf(tty_fd, "history: %zu\n", history.size());
	history_index = 0;
}

/**
 * @brief Clear the history
 * 
 */
void
tm_rl_clear_history(void)
{
	history.clear();
	history_index = 0;
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