/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handlers.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 11:26:53 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 12:42:24 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "unix_socket/UnixSocket.hpp"

/**
 * Communication format, string based:
 * 
 * Name: <command_name> CRLF
 * Args: <arg1> <arg2> ... CRLF
 * Opts: <option1> <option2> ... CRLF
 * CRLF
 */

static inline std::string
_no_args_handler(const std::vector<std::string>& args)
{
	std::string payload = "Name: " + args[0] + TM_CRLF;
	payload += TM_CRLF; // End of command

	return (payload);
}

static inline std::string
_procs_handler(const std::vector<std::string>& args)
{
	std::string payload = "Name: " + args[0] + TM_CRLF;
	
	if (args.size() > 1)
	{
		payload += "Args: ";
		for (size_t i = 1; i < args.size(); ++i)
		{
			payload += args[i];
			if (i < args.size() - 1)
				payload += " ";
		}
		payload += TM_CRLF;
	}
	else
	{
		throw std::invalid_argument("Invalid usage");
	}
	payload += TM_CRLF; // End of command

	return (payload);
}

static inline std::string
_procs_or_daemon_handler(const std::vector<std::string>& args)
{
	std::string payload = "Name: " + args[0] + TM_CRLF;
	
	if (args.size() > 1)
	{
		payload += "Args: ";
		for (size_t i = 1; i < args.size(); ++i)
		{
			payload += args[i];
			if (i < args.size() - 1)
				payload += " ";
		}
		payload += TM_CRLF;
	}
	else
	{
		payload += "Args:" TM_CRLF;
	}
	payload += TM_CRLF; // End of command

	return (payload);
}

static inline std::string
_signal_handler(const std::vector<std::string>& args)
{
	std::string payload = "Name: " + args[0] + TM_CRLF;
	
	if (args.size() > 2)
	{
		payload += "Opts: " + args[1] + TM_CRLF; // Signal number
		payload += "Args: ";
		for (size_t i = 1; i < args.size(); ++i)
		{
			payload += args[i];
			if (i < args.size() - 1)
				payload += " ";
		}
		payload += TM_CRLF;
	}
	else
	{
		throw std::invalid_argument("Invalid usage");
	}
	payload += TM_CRLF; // End of command

	return (payload);
}

static inline std::string
_tail_handler(const std::vector<std::string>& args)
{
	std::string payload = "Name: " + args[0] + TM_CRLF;

	if (args.size() < 2 || args.size() > 3)
	{
		throw std::invalid_argument("Invalid usage");
	}
	else
	{
		payload += "Args: " + args[1] + TM_CRLF; // Process name
		if (args.size() == 3)
		{
			payload += "Opts: " + args[2] + TM_CRLF; // Optional file descriptor
		}
	}
	payload += TM_CRLF; // End of command

	return (payload);
}

extern const std::unordered_map<std::string, std::string (*)(const std::vector<std::string>&)> command_handler = {
	{"add", &_procs_handler},
	{"avail", &_no_args_handler},
	{"clear", &_procs_handler},
	{"exit", &_no_args_handler},
	{"maintail", &_no_args_handler},
	{"pid", &_procs_or_daemon_handler},
	{"reload", &_no_args_handler},
	{"remove", &_procs_handler},
	{"reread", &_no_args_handler},
	{"restart", &_procs_handler},
	{"shutdown", &_no_args_handler},
	{"signal", &_signal_handler},
	{"start", &_procs_handler},
	{"status", &_procs_or_daemon_handler},
	{"stop", &_procs_handler},
	{"tail", &_tail_handler},
	{"update", &_no_args_handler},
	{"version", &_no_args_handler}
};