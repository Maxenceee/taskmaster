/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/04/20 16:22:09 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "logger/Logger.hpp"
#include <optional>
#include "readline.hpp"
#include "utils/utils.hpp"
#include "signal.hpp"
#include <readline/readline.h>
#include <readline/history.h>

extern int tty_fd;

struct CommandNodeUsage {
	std::string usage;
	std::string description;
};

struct CommandNode {
	std::string								name;							// Nom de la commande
	std::vector<struct CommandNodeUsage> 	usages;							// Liste des variantes d'usage de la commande
	bool									visible;						// Indique si la commande doit être affichée dans l'aide
	bool									ripple_autocomplete;			// Indique si l'autocomplétion doit être propagée aux sous-commandes
	char*									(*suggest)(const char*, int);	// Pointeur vers la fonction de rappel associée à la commande
};

char*
get_process_name(const char* text, int state)
{
	static std::vector<std::string> suggestions;
	static int index = 0;
	static size_t len;

(void)dprintf(tty_fd, "text: %s, state: %d\n", text, state);

	if (state == 0)
	{
		index = 0;
		len = strlen(text);
		suggestions.clear();

		suggestions.push_back("child");
		suggestions.push_back("child_key");
	}

	while (index < suggestions.size()) {
		const std::string& cmd = suggestions[index];
		index++;

		if (cmd.compare(0, len, text) == 0) {
			return strdup(cmd.c_str());
		}
	}

	return nullptr;
}

const std::vector<CommandNode> commands = {
	{"add", {
		{"add <name> [...]", "Activates any updates in config for process/group"},
	}, true, false, &get_process_name},
	{"avail", {
		{"avail", "Display all configured processes"},
	}, true, false, &get_process_name},
	{"clear", {
		{"clear <name>", "Clear a process' log files."},
		{"clear <name> <name>", "Clear multiple process' log files"},
		{"clear all", "Clear all process' log files"},
	}, true, false, &get_process_name},
	{"exit", {
		{"exit", "Exit the " TM_PROJECT " shell."},
	}, true, false, &get_process_name},
	{"maintail", {
		{"maintail", "tail of " TM_PROJECT " main log file"},
	}, true, false, &get_process_name},
	{"pid", {
		{"pid", "Get the PID of " TM_PROJECTD "."},
		{"pid <name>", "Get the PID of a single child process by name."},
		{"pid all", "Get the PID of every child process, one per line."},
	}, true, false, &get_process_name},
	{"reload", {
		{"reload", "Restart the remote " TM_PROJECTD "."},
	}, true, false, &get_process_name},
	{"remove", {
		{"remove <name> [...]", "Removes process from active config"},
	}, true, false, &get_process_name},
	{"reread", {
		{"reread", "Reload the daemon's configuration files without add/remove"},
	}, true, false, &get_process_name},
	{"restart", {
		{"restart <name>", "Restart a process"},
		{"restart <name> <name>", "Restart multiple processes"},
		{"restart all", "Restart all processes"},
	}, true, false, &get_process_name},
	{"shutdown", {
		{"shutdown", "Shut the remote " TM_PROJECTD " down."},
	}, true, false, &get_process_name},
	{"signal", {
		{"signal <signal name> <name>", "Signal a process"},
		{"signal <signal name> <name> <name>", "Signal multiple processes"},
		{"signal <signal name> all", "Signal all processes"},
	}, true, false, &get_process_name},
	{"start", {
		{"start <name>", "Start a process"},
		{"start <name> <name>", "Start multiple processes"},
		{"start all", "Start all processes"},
	}, true, false, &get_process_name},
	{"status", {
		{"status <name>", "Get status for a single process"},
		{"status <name> <name>", "Get status for multiple named processes"},
		{"status", "Get all process status info"},
	}, true, false, &get_process_name},
	{"stop", {
		{"stop <name>", "Stop a process"},
		{"stop <name> <name>", "Stop multiple processes"},
		{"stop all", "Stop all processes"},
	}, true, false, &get_process_name},
	{"tail", {
		{"tail <name> [stdout|stderr] (default stdout)", ""},
	}, true, false, &get_process_name},
	{"update", {
		{"update", "Reload config and add/remove as necessary, and will restart affected programs"},
	}, true, false, &get_process_name},
	{"version", {
		{"version", "Show the version of the remote " TM_PROJECTD " process"},
	}, true, false, &get_process_name},
	{"help", {
		{"help", "Show help"},
		{"help <command>", "Show help for a specific command"},
	}, false, true, nullptr},
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
	size_t maxlen = 0;

	for (const auto& command : commands) {
		if (command.name == cmd) {
			for (const auto& usage : command.usages) {
				if (usage.usage.length() > maxlen) {
					maxlen = usage.usage.length();
				}
			}
			maxlen += 2; // Ajouter un espace pour l'alignement
			for (const auto& usage : command.usages) {
				std::cout << std::setw(maxlen) << std::left << usage.usage << usage.description << "\n";
			}
			return;
		}
	}
	std::cout << "*** No help on " << cmd << "\n";
}

char *
command_generator(const char *text, int state)
{
	static size_t index;
	static size_t len;

(void)dprintf(tty_fd, "text: %s, state: %d\n", text, state);

	if (state == 0) { // Réinitialisation lors du premier appel
		index = 0;
		len = strlen(text);
	}

	// Parcours des commandes pour trouver des correspondances
	while (index < commands.size()) {
		const std::string& cmd = commands[index].name;
		index++;

		if (cmd.compare(0, len, text) == 0) {
			return strdup(cmd.c_str()); // Correspondance trouvée
		}
	}

	return nullptr; // Plus de suggestions
}

static char**
autocomplete(const char *text, int start, int end)
{
	(void)end;

(void)dprintf(tty_fd, "text: %s, %d, %d\n", text, start, end);

	rl_attempted_completion_over = 1;

	if (start == 0)
	{
		return rl_completion_matches(text, command_generator);
	}

	std::string cmd = std::string(rl_line_buffer).substr(0, std::string(rl_line_buffer).find(' '));
	
(void)dprintf(tty_fd, "rl_line_buffer: %s\n", rl_line_buffer);
(void)dprintf(tty_fd, "cmd: %s\n", cmd.c_str());

	// Chercher la commande correspondante dans `commands`
	for (const auto& node : commands) {
		if (node.name == cmd && node.suggest) {
			// On retourne une fonction de complétion spécifique pour les noms de processus
			return rl_completion_matches(text, node.suggest);
		}
	}

	return nullptr;
}

static void
interruptHandler(int sig_int)
{
	(void)sig_int;
	// tm_rl_new_line();
	printf("\n");
	rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
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
		(void)close(sockfd);
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

	(void)memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	(void)strncpy(servaddr.sun_path, socket_path.c_str(), sizeof(servaddr.sun_path) - 1);

	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		std::cout << "Cannot connect to the Taskmaster daemon at " << unix_path << ". Is the daemon running?" << std::endl;
		(void)close(sockfd);
		return (-1);
	}

	return sockfd;
}

void
attach_readline()
{
	// tm_rl_add_autocomplete_handler([](const std::string& input, size_t cursor_pos) {
	// 	auto tokens = tokenize(input);

	// 	return autocomplete(commands, tokens, cursor_pos);
	// });

	rl_attempted_completion_function = autocomplete;

	setup_signal(SIGINT, interruptHandler);
	setup_signal(SIGPIPE, SIG_IGN);

	size_t total_sent = 0;
	size_t total_recv = 0;

	do
	{
		// auto rl_in = tm_readline(TM_PROJECTCTL "> ");
		char *rl_in = readline(TM_PROJECTCTL "> ");
		if (NULL == rl_in)
		{
			break;
		}

		if (!*rl_in)
		{
			continue;
		}

		// Check if the current input is the same as the last history entry
		HIST_ENTRY *last_entry = history_get(history_length);
		if (!last_entry || strcmp(last_entry->line, rl_in) != 0) {
			add_history(rl_in);
		}

		try
		{
			std::vector<std::string> tokens = tokenize(rl_in);
			free(rl_in);
			if (tokens.empty()) {
				continue;
			}

			if (tokens[0] == "exit") {
				break;
			}

			if (tokens[0] == "help") {
				if (tokens.size() == 2) {
					show_command_info(tokens[1]);
				} else {
					show_help();
				}
				continue;
			}

			int socket_fd = connect_server(TM_SOCKET_PATH);
			if (socket_fd == -1) {
				continue;
			}

			std::string message = join(tokens, " ");

			// std::cout << "Input: (" << input << ")" << std::endl;
			total_sent += send_message(socket_fd, message.c_str(), message.length());
			total_recv += read_message(socket_fd);
			close(socket_fd);
		}
		catch(...)
		{
			std::cerr << "*** Invalid command\n";
			rl_on_new_line();
		}
		
	} while (true);
}

int
main(int argc, char* const* argv)
{
	Logger::printHeader();

	if (argc != 1) {
		std::cerr << "Usage: " << argv[0] << std::endl;
		return (TM_FAILURE);
	}

	Logger::init("Starting " TM_PROJECTCTL);
	Logger::setDebug(true);

	try {
		attach_readline();
	} catch (const std::exception& e) {
		Logger::error(e.what());
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}