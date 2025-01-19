/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:07:36 by mgama             #+#    #+#             */
/*   Updated: 2025/01/19 21:58:55 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "readline.hpp"

int tty_fd = open("/dev/ttys007", O_RDWR);

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
	case 'b':
		move_cursor_by_word(input_buffer, cursor_pos, false);
		return;
	case 'f':
		move_cursor_by_word(input_buffer, cursor_pos, true);
		return;
	case 100:
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
	case 'C':
		if (cursor_pos < input_buffer.size()) {
			cursor_pos++;
			std::cout << "\033[C";  // Déplacer le curseur à droite
		}
		break;
	case 'D':
		if (cursor_pos > 0) {
			cursor_pos--;
			std::cout << "\033[D";  // Déplacer le curseur à gauche
		}
		break;
	case 51:
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
	case 21:
		input_buffer.clear();
		cursor_pos = 0;
		draw_line(prompt, input_buffer, cursor_pos);
		break;
	case 23:
		delete_word(input_buffer, cursor_pos, false);
		draw_line(prompt, input_buffer, cursor_pos);
		break;
	case 27:
		escape_sequence(prompt, input_buffer, cursor_pos);
		break;
	case 8:
	case 127:
		left_suppr(prompt, input_buffer, cursor_pos);
		break;
	case '\r':
		cursor_pos = 0;
		break;
	case 3:
	case '\n':
		std::cout << std::endl;
		return 0;
	case '\t':
		break;
	default:
		add_char(prompt, input_buffer, cursor_pos, ch);
		break;
	}
	return 1;
}

std::string
readline(const std::string& prompt)
{
	std::vector<char> input_buffer;
	int cursor_pos = 0;

	draw_line(prompt, input_buffer, cursor_pos);

	while (process_input(prompt, input_buffer, cursor_pos)) {
		;
	}

	return std::string(input_buffer.begin(), input_buffer.end());
}