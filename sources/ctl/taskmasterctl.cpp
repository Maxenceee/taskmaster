/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/01/31 15:34:27 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include <optional>
#include "readline.hpp"
#include "utils/utils.hpp"
#include "signal.hpp"

extern int tty_fd;

struct CommandNode {
	std::string					name;			// Nom de la commande
	std::string					description;	// Description pour l'aide
	std::vector<std::string>	arguments;		// Liste des arguments valides
	std::vector<CommandNode>	children;		// Sous-commandes
};

std::vector<CommandNode> commands = {
    {"list", "List tasks", {}, {
		{"processes", "Manage subtasks", {}, {}},
		{"pelements", "Manage subtasks", {}, {}}
	}},
    {"add", "Add a new task", {"--name", "--due"}, {
		{"subtask", "Manage subtasks", {}, {}}
	}},
    {"remove", "Remove a task", {"--id"}, {}},
};

bool
validate_command(const CommandNode& node, const std::vector<std::string>& tokens, size_t index = 0)
{
	if (index >= tokens.size()) return true;

	for (const auto& child : node.children) {
		if (child.name == tokens[index]) {
			return validate_command(child, tokens, index + 1);
		}
	}

	return false;  // Commande invalide si aucun enfant correspondant n'est trouvé
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

	// Identifier le niveau de la commande à partir des tokens précédents
	const std::vector<CommandNode>* current_level = &commands;

	for (size_t i = 0; i < word_start; ++i) {
		const std::string& token = tokens[i];
		bool found = false;

		for (const auto& command : *current_level) {
			if (command.name == token) {
				current_level = &command.children;
				found = true;
				break;
			}
		}

		if (!found) {
			return {}; // Si un chemin invalide est trouvé, aucune suggestion possible
		}
	}

	// Ajouter les suggestions basées sur le préfixe
	for (const auto& command : *current_level) {
		if (command.name.compare(0, prefix.size(), prefix) == 0) {
			suggestions.push_back(command.name);
		}
	}

	for (auto& command : *current_level) {
dprintf(tty_fd, "command: %s\n", command.name.c_str());
	}

	return suggestions;
}

void
send_message(int sockfd, const char* message)
{
	if (send(sockfd, message, strlen(message), 0) < 0) {
		perror("send failed");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}

static void
interruptHandler(int sig_int)
{
	(void)sig_int;
	tm_rl_new_line();
}

int
main(int argc, char* const* argv)
{
	// if (argc < 2 || argc > 3) {
	// 	std::cerr << "Usage: " << argv[0] << " <message>" << std::endl;
	// 	return 1;
	// }

	// int sockfd;
	// struct sockaddr_un servaddr;

	// // Create socket
	// if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	// 	perror("socket creation failed");
	// 	exit(EXIT_FAILURE);
	// }

	// // Set server address
	// memset(&servaddr, 0, sizeof(servaddr));
	// servaddr.sun_family = AF_UNIX;
	// strncpy(servaddr.sun_path, TM_SOCKET_PATH, sizeof(servaddr.sun_path) - 1);

	// // Connect to server
	// if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
	// 	perror("connect failed");
	// 	close(sockfd);
	// 	exit(EXIT_FAILURE);
	// }

	tm_rl_add_autocomplete_handler([](const std::string& input, size_t cursor_pos) {
		auto tokens = tokenize(input);

		return autocomplete(commands, tokens, cursor_pos);
	});

	do
	{
		setup_signal(SIGINT, interruptHandler);

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

		std::cout << "Input: (" << input << ")" << std::endl;
	} while (true);
	// send_message(sockfd, argv[1]);

	// Receive a message from the server
	// char buffer[1024];
	// int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
	// if (n < 0) {
	// 	perror("recv failed");
	// 	close(sockfd);
	// 	exit(EXIT_FAILURE);
	// }
	// buffer[n] = '\0';
	// std::cout << "Server: " << buffer << std::endl;

	// // Close the socket
	// close(sockfd);
	return 0;
}