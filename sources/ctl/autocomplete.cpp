/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   autocomplete.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 13:24:58 by mgama             #+#    #+#             */
/*   Updated: 2025/05/30 16:13:50 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "autocomplete.hpp"

extern int tty_fd;

const std::vector<tm_CommandNode> commands = {
	{"add", {
		{"add <name> [...]", "Activates any updates in config for process/group"},
	}, true, false, &get_process_avail_name},
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
	}, true, false, &get_process_avail_name},
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

void
show_help()
{
	std::cout << "\n";
	std::cout << "default commands (type help <topic>):" << "\n";
	std::cout << "=====================================" << "\n";

	const int columns = 5;

	// Filtrer les commandes visibles
	std::vector<tm_CommandNode> visibleCommands;
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

void
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

static char *
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

char**
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
