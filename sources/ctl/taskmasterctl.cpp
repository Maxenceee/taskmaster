/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 13:30:36 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "logger/Logger.hpp"
#include "readline.hpp"
#include "autocomplete.hpp"
#include "utils/utils.hpp"
#include "client/UnixSocketClient.hpp"
#include "signal.hpp"

char*
get_process_name(const char* text, int state)
{
	static std::vector<std::string> suggestions;
	static int index = 0;
	static size_t len;

	if (state == 0)
	{
		index = 0;
		len = strlen(text);
		suggestions.clear();

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

static void
interruptHandler(int sig_int)
{
	(void)sig_int;
	std::cout << "\n";
	rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

static void
interruptHandlerWhenWorking(int sig_int)
{
	(void)sig_int;
	rl_on_new_line();
}

static int
handle_stdin_input(void)
{
	std::string rl_in;
	std::getline(std::cin, rl_in);

	if (rl_in.empty()) {
		return (TM_SUCCESS);
	}
	
	std::vector<std::string> tokens = tokenize(rl_in);
	if (tokens.empty()) {
	}

	if (tokens[0] == "help") {
		if (tokens.size() == 2) {
			show_command_info(tokens[1]);
		} else {
			show_help();
		}
		return (TM_SUCCESS);
	}

	UnixSocketClient client(TM_SOCKET_PATH);
	if (client.connect() == TM_FAILURE)
	{
		std::cout << "Cannot connect to the Taskmaster daemon at unix://" << client.getSocketPath() << ". Is the daemon running?" << std::endl;
		return (TM_FAILURE);
	}

	(void)client.sendCmd(tokens);
	(void)client.recv();

	return (TM_SUCCESS);
}

static void
attach_readline()
{
	rl_attempted_completion_function = autocomplete;

	setup_signal(SIGPIPE, SIG_IGN);

	size_t total_sent = 0;
	size_t total_recv = 0;

	do
	{
		setup_signal(SIGINT, interruptHandler);

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

		setup_signal(SIGINT, interruptHandlerWhenWorking);

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

			UnixSocketClient client(TM_SOCKET_PATH);
			if (client.connect() == TM_FAILURE)
			{
				std::cout << "Cannot connect to the Taskmaster daemon at unix://" << client.getSocketPath() << ". Is the daemon running?" << std::endl;
				continue;
			}

			(void)client.sendCmd(tokens);
			(void)client.recv();
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

	if (reopenstds() == -1) {
		Logger::perror("Failed to reopen stds");
		return (TM_FAILURE);
	}

	Logger::init("Starting " TM_PROJECTCTL);
	Logger::setDebug(true);

	if (isatty(STDIN_FILENO) == 0) {
		return (handle_stdin_input());
	}

	try {
		attach_readline();
	} catch (const std::exception& e) {
		Logger::error(e.what());
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}