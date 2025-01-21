/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/01/21 18:19:08 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include <optional>
#include "readline.hpp"
#include "utils/utils.hpp"
#include "signal.hpp"

struct CommandNode {
	std::string name;  // Nom de la commande
	std::string description;  // Description pour l'aide
	std::vector<std::string> arguments;  // Liste des arguments valides
	std::vector<CommandNode> children;  // Sous-commandes
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

	// Trouver à quel mot correspond le curseur
	size_t token_index = 0;
	size_t char_count = 0;

	for (size_t i = 0; i < tokens.size(); ++i) {
		char_count += tokens[i].size();
		if (cursor_pos <= char_count) {
			token_index = i;
			break;
		}
		char_count++; // Compte l'espace entre les tokens
	}

	// Si le curseur est après le dernier mot et après un espace, suggérer des sous-commandes
	if (cursor_pos == char_count && cursor_pos > 0 && tokens.back().back() == ' ') {
		token_index = tokens.size(); // Nouveau mot
	}

	// Commencer la recherche
	const std::vector<CommandNode>* current_level = &commands;

	for (size_t i = 0; i < token_index; ++i) {
		bool found = false;

		for (const auto& command : *current_level) {
			if (command.name == tokens[i]) {
				// Descendre dans les sous-commandes si correspondance trouvée
				current_level = &command.children;
				found = true;
				break;
			}
		}

		if (!found) {
			// Si un token ne correspond pas, retourner aucune suggestion
			return {};
		}
	}

	// Si le curseur est sur un mot existant, suggérer les complétions possibles
	if (token_index < tokens.size()) {
		const std::string& current_token = tokens[token_index];
		for (const auto& command : *current_level) {
			if (command.name.rfind(current_token, 0) == 0) { // Correspondance au début du mot
				suggestions.push_back(command.name);
			}
		}
	} else {
		// Sinon, suggérer toutes les commandes à ce niveau
		for (const auto& command : *current_level) {
			suggestions.push_back(command.name);
		}
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

static void interruptHandler(int sig_int)
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
		if (tokens.empty()) {
			return std::vector<std::string>();
		}

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