/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/02/03 10:55:13 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "logger/Logger.hpp"
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

	return false;
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
		perror("recv failed");
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
		perror("socket creation failed");
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