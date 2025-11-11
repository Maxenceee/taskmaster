/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/11/11 12:04:31 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "getopt.hpp"
#include "readline.hpp"
#include "autocomplete.hpp"
#include "signal.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"
#include "client/UnixSocketClient.hpp"

extern int tty_fd;

extern const std::unordered_map<std::string, std::string (*)(const std::vector<std::string>&)> command_handler;

std::string	socket_path = TM_SOCKET_PATH;

static void
usage(char const* exec)
{
	std::cout << TM_PROJECTCTL " -- control applications run by taskmasterd from the cmd line" << "\n\n";
	std::cout << "Usage: " << exec << " [options] [action [arguments]]" << "\n\n";
	std::cout << "Options:" << "\n";
	std::cout << "  " << "-h" << ", " << std::left << std::setw(20) << "--help" << " Display this help and exit" << "\n";
	std::cout << "  " << "-i" << ", " << std::left << std::setw(20) << "--interactive" << " start an interactive shell after executing commands" << "\n";
	std::cout << "  " << "-s" << ", " << std::left << std::setw(20) << "--socket <path>" << " Specify the unix socket path to connect to" << "\n";
	std::cout << "\n" << "action [arguments] -- see below" << "\n\n";
	std::cout << "Actions are commands like \"tail\" or \"stop\".  If -i is specified or no action is\n";
	std::cout << "specified on the command line, a \"shell\" interpreting actions typed\n";
	std::cout << "interactively is started.  Use the action \"help\" to find out about available\n";
	std::cout << "actions.\n";
	exit(64);
}

char*
get_process_avail_name(const char* text, int state)
{
	static std::vector<std::string> suggestions;
	static size_t index = 0;
	static size_t len;

	if (state == 0)
	{
		index = 0;
		len = strlen(text);
		suggestions.clear();

		UnixSocketClient client(socket_path.c_str());
		if (client.connect() == TM_FAILURE)
		{
			return nullptr;
		}

		(void)client.send("Name: internal" TM_CRLF "Args: processes" TM_CRLF "Opts: avail" TM_CRLF TM_CRLF);
		std::string buffer = client.recv();

		if (buffer.empty())
		{
			return nullptr;
		}

		size_t pos = 0;
		while ((pos = buffer.find(TM_CRLF)) != std::string::npos)
		{
			std::string line = buffer.substr(0, pos);
			buffer.erase(0, pos + 2);

			suggestions.push_back(line);
		}
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

char*
get_process_name(const char* text, int state)
{
	static std::vector<std::string> suggestions;
	static size_t index = 0;
	static size_t len;

	if (state == 0)
	{
		index = 0;
		len = strlen(text);
		suggestions.clear();

		UnixSocketClient client(socket_path.c_str());
		if (client.connect() == TM_FAILURE)
		{
			return nullptr;
		}

		(void)client.send("Name: internal" TM_CRLF "Args: processes" TM_CRLF "Opts: procs" TM_CRLF TM_CRLF);
		std::string buffer = client.recv();

		if (buffer.empty())
		{
			return nullptr;
		}

		size_t pos = 0;
		while ((pos = buffer.find(TM_CRLF)) != std::string::npos)
		{
			std::string line = buffer.substr(0, pos);
			buffer.erase(0, pos + 2);

			suggestions.push_back(line);
		}
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
interruptHandler(int sig_int __unused)
{
	std::cout << "\n";
	rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

static void
interruptHandlerWhenWorking(int sig_int __unused)
{
	rl_on_new_line();
}

static int
send_cmd(const std::vector<std::string>& tokens)
{
	if (tokens[0] == "help") {
		if (tokens.size() == 2) {
			show_command_info(tokens[1]);
		} else {
			show_help();
		}
		return (TM_SUCCESS);
	}

	auto exists = command_handler.count(tokens[0]);
	if (exists == 0)
	{
		std::cerr << "*** Unknown command" << "\n";
		rl_on_new_line();
		return (TM_FAILURE);
	}

	auto handler = command_handler.at(tokens[0]);

	std::string payload = handler(tokens);

	UnixSocketClient client(socket_path.c_str());
	if (client.connect() == TM_FAILURE)
	{
		std::cout << "Cannot connect to the Taskmaster daemon at unix://" << client.getSocketPath() << ". Is the daemon running?" << std::endl;
		return (TM_FAILURE);
	}

	(void)client.send(payload);
	(void)client.print(true);
	return (TM_SUCCESS);
}

static void
attach_readline()
{
	rl_attempted_completion_function = autocomplete;

	setup_signal(SIGPIPE, SIG_IGN);

	do
	{
		setup_signal(SIGINT, interruptHandler);

		char *rl_in = readline(RL_PROMPT);
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

			if (tokens[0] == "exit")
			{
				break;
			}

			(void)send_cmd(tokens);
		}
		catch(...)
		{
			std::cerr << "*** Invalid command" << "\n";
			rl_on_new_line();
		}
		
	} while (true);
}

int
main(int argc, char* const* argv)
{
	Logger::printHeader();

	int ch = 0;
	bool interactive = false;

	struct tm_getopt_list_s optlist[] = {
		{"interactive", 'i', TM_OPTPARSE_NONE},
		{"socket", 's', TM_OPTPARSE_REQUIRED},
		{"help", 'h', TM_OPTPARSE_NONE},
		{"version", 'v', TM_OPTPARSE_NONE},
		{nullptr, 0, TM_OPTPARSE_NONE}
	};
	struct tm_getopt_s options;

	tm_getopt_init(&options, argv);
	while ((ch = tm_getopt(&options, optlist, NULL)) != -1)
	{
		switch (ch)
		{
			case 'i':
				interactive = true;
				break;
			case 's':
				if (options.optarg == nullptr)
				{
					Logger::error("No socket path specified");
					return (TM_FAILURE);
				}
				socket_path = resolve_path(std::string(options.optarg), "unix://");
				break;
			case 'v':
				std::cout << B_PINK << TM_PROJECTCTL " " B_CYAN TM_VERSION B_PINK " by " B_CYAN TM_AUTHOR RESET << std::endl;
				exit(0);
			case 'h':
			default:
				usage(argv[0]);
		}
	}

	std::vector<std::string> remaining_args;
	if (argc - options.optind == 0)
	{
		interactive = true;
	}
	else
	{
		for (int i = options.optind; i < argc; ++i)
			remaining_args.push_back(options.argv[i]);
	}

	if (reopenstds() == -1) {
		Logger::perror("Failed to reopen stds");
		return (TM_FAILURE);
	}

	Logger::init("Starting " TM_PROJECTCTL);

	try {
		if (false == remaining_args.empty())
		{
			(void)send_cmd(remaining_args);
		}
		if (interactive)
		{
			attach_readline();
		}
	} catch (const std::exception& e) {
		Logger::error(e.what());
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}
