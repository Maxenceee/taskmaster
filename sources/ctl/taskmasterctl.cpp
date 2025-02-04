/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/02/04 11:04:04 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "logger/Logger.hpp"
#include <optional>
#include "readline.hpp"
#include "utils/utils.hpp"
#include "signal.hpp"

extern int tty_fd;

struct CommandNodeUsage {
	std::string usage;
	std::string description;
};

struct CommandNode {
	std::string								name;				// Nom de la commande
	std::vector<struct CommandNodeUsage> 	usages;				// Liste des variantes d'usage de la commande
	bool									visible;			// Indique si la commande doit être affichée dans l'aide
	bool									ripple_autocomplete;// Indique si l'autocomplétion doit être propagée aux sous-commandes
};

const std::vector<CommandNode> commands = {
	{"list", {
		{"list", "List all processes"},
		{"list <name>", "List a specific process"},
	}, true, false},
	{"add", {
		{"add <name> <cmd>", "Add a new process"},
		{"add <name> <cmd> <numprocs>", "Add a new process with a specific number of instances"},
	}, true, false},
	{"remove", {
		{"remove <name>", "Remove a process"},
	}, true, false},
	{"help", {
		{"help", "Show help"},
		{"help <command>", "Show help for a specific command"},
	}, false, true},
};

static void
show_help()
{
	std::cout << "\n";
	std::cout << "default commands (type help <topic>):" << "\n";
	std::cout << "=====================================" << "\n";

	const int columns = 5;

	// Filtrer les commandes visibles
	std::vector<CommandNode> visibleCommands;
	for (const auto& command : commands) {
		if (command.visible) {
			visibleCommands.push_back(command);
		}
	}

	size_t total = visibleCommands.size();
	size_t rows = (total + columns - 1) / columns; // Nombre de lignes nécessaires

	// Calcul de la largeur maximale pour chaque colonne
	std::vector<size_t> colWidths(columns, 0);

	for (size_t col = 0; col < columns; ++col) {
		for (size_t row = 0; row < rows; ++row) {
			size_t index = row * columns + col;
			if (index < total) {
				colWidths[col] = std::max(colWidths[col], visibleCommands[index].name.size());
			}
		}
	}

	// Affichage des commandes avec un alignement dynamique
	for (size_t row = 0; row < rows; ++row) {
		for (size_t col = 0; col < columns; ++col) {
			size_t index = row * columns + col;
			if (index < total) {
				std::cout << std::left << std::setw(colWidths[col] + 2) << visibleCommands[index].name;
			}
		}
		std::cout << "\n";
	}

	std::cout << std::endl;
}

static void
show_command_info(const std::string &cmd)
{
	for (const auto& command : commands) {
		if (command.name == cmd) {
			for (const auto& usage : command.usages) {
				std::cout << std::setw(17) << std::left << usage.usage << usage.description << "\n";
			}
			return;
		}
	}
	std::cout << "*** No help on " << cmd << "\n";
}

std::vector<std::string>
autocomplete(const std::vector<CommandNode>& commands, const std::vector<std::string>& tokens, size_t cursor_pos)
{
	std::vector<std::string> suggestions;

	if (tokens.empty()) {
		for (const auto& command : commands) {
			suggestions.push_back(command.name);
		}
		return suggestions;
	}

	// Calculer la position du mot actuellement ciblé par le curseur
	size_t word_start = 0, word_end = 0;
	bool found_word = false;

	size_t current_pos = 0;

	// Déterminer le mot en cours d'édition
	for (size_t i = 0, len = 0; i < tokens.size(); ++i) {
		len += tokens[i].length();
		if (cursor_pos <= len + i) { // Ajouter i pour les espaces séparateurs
			word_start = i;
			word_end = i;
			found_word = true;
			break;
		}
	}

	// Si aucun mot trouvé, on est sur un nouveau mot
	if (!found_word) {
		return {};
	}

	// Le mot en cours d'édition
	std::string current_word = tokens[word_start];
	std::string prefix = current_word.substr(0, cursor_pos - word_start);

	for (const auto& command : commands) {
        if (command.name.compare(0, prefix.size(), prefix) == 0) {
            suggestions.push_back(command.name);
        }
    }

	return suggestions;
}

static void
interruptHandler(int sig_int)
{
	(void)sig_int;
	tm_rl_new_line();
}

ssize_t
send_message(int sockfd, const char* message, size_t strlen)
{
	return send(sockfd, message, strlen, 0);
}

ssize_t
read_message(int sockfd)
{
	char buffer[1024];

	ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
	if (n < 0) {
		Logger::perror("recv failed");
		close(sockfd);
		return (-1);
	}
	buffer[n] = '\0';
	std::cout << "Server: " << buffer << std::endl;
	return (n);
}

int
connect_server(const std::string& unix_path)
{
	int sockfd;
	struct sockaddr_un servaddr;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		Logger::perror("socket creation failed");
		return (-1);
	}

	const auto socket_path = resolve_path(unix_path, "unix://");

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strncpy(servaddr.sun_path, socket_path.c_str(), sizeof(servaddr.sun_path) - 1);

	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		std::cout << "Cannot connect to the Taskmaster daemon at " << unix_path << ". Is the daemon running?" << std::endl;
		close(sockfd);
		return (-1);
	}

	return sockfd;
}

void
attach_readline()
{
	tm_rl_add_autocomplete_handler([](const std::string& input, size_t cursor_pos) {
		auto tokens = tokenize(input);

		return autocomplete(commands, tokens, cursor_pos);
	});

	setup_signal(SIGINT, interruptHandler);
	setup_signal(SIGPIPE, SIG_IGN);

	size_t total_sent = 0;
	size_t total_recv = 0;

	do
	{
		auto rl_in = tm_readline("taskmasterctl> ");
		if (!rl_in) {
			break;
		}
		auto input = rl_in.value();

		tm_rl_add_history(input);
		trim(input);
		if (input == "exit") {
			break;
		}

		std::vector<std::string> tokens = tokenize(input);
		if (tokens.empty()) {
			continue;
		}

		if (tokens[0] == "help") {
			if (tokens.size() > 1) {
				show_command_info(tokens[1]);
			} else {
				show_help();
			}
			continue;
		}

		if (input.empty()) {
			continue;
		}

		int socket_fd = connect_server(TM_SOCKET_PATH);
		if (socket_fd == -1) {
			continue;
		}

		// std::cout << "Input: (" << input << ")" << std::endl;
		total_sent += send_message(socket_fd, input.c_str(), input.length());
		total_recv += read_message(socket_fd);
		close(socket_fd);
	} while (true);
}

int
main(int argc, char* const* argv)
{
	std::cout << "\n" << HEADER << TM_OCTO << RESET << "\n";
	std::cout << HACKER << std::setw(12) << "" << "Taskmaster" << RESET << "\n" << std::endl;

	if (argc != 1) {
		std::cerr << "Usage: " << argv[0] << std::endl;
		return (TM_FAILURE);
	}

	Logger::init();
	Logger::setDebug(true);

	try {
		attach_readline();
	} catch (const std::exception& e) {
		Logger::error(e.what());
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}