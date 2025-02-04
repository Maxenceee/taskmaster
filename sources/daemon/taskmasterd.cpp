/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/02/04 11:01:35 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "spawn.hpp"
#include "signal.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"
#include "unix_socket/server/UnixSocketServer.hpp"

bool running = true;

void
interruptHandler(int sig_int)
{
	(void)sig_int;
	std::cout << "\b\b"; // rm ^C from tty
	Logger::print("Signal received: " + std::string(strsignal(sig_int)), B_GREEN);
	running = false;
}

int
create_pid_file(void)
{
	int32_t	tm_pid;
	int		pid_fd;
	char	pid_str[TM_INT64_LEN + 2];
	int		len;

	tm_pid = getpid();

	pid_fd = open(TM_PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, TM_DEFAULT_FILE_MODE);
	if (pid_fd == -1)
	{
		Logger::perror("open");
		return (TM_FAILURE);
	}

	len = snprintf(pid_str, TM_INT64_LEN + 2, "%i", tm_pid);
	if (len == -1)
	{
		Logger::perror("snprintf");
		return (TM_FAILURE);
	}
	if (write(pid_fd, pid_str, len) == -1)
	{
		Logger::perror("write");
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}

int
check_pid_file(void)
{
	int		pid_fd;
	char	pid_str[TM_INT64_LEN + 2];
	ssize_t	len;
	pid_t	pid;

	pid_fd = open(TM_PID_FILE, O_RDONLY);
	if (pid_fd == -1)
	{
		if (errno == ENOENT)
		{
			return (TM_SUCCESS);
		}
		Logger::perror("open");
		return (TM_FAILURE);
	}

	len = read(pid_fd, pid_str, TM_INT64_LEN + 2);
	if (len == -1)
	{
		Logger::perror("read");
		return (TM_FAILURE);
	}

	pid = atoi(pid_str);
	if (pid < 1)
	{
		return (TM_SUCCESS);
	}
	if (kill(pid, 0) == -1)
	{
		if (errno == ESRCH)
		{
			return (TM_SUCCESS);
		}
		Logger::perror("kill");
		return (TM_FAILURE);
	}

	return (TM_FAILURE);
}

void
start_server(void)
{
	UnixSocketServer server(TM_SOCKET_PATH);
	server.listen();

	setup_signal(SIGPIPE, SIG_IGN);
	
	do
	{
		if (server.cycle())
		{
			break;
		}
	} while (running);
}

int
main(int argc, char* const* argv, char* const* envp)
{
	std::cout << "\n" << HEADER << TM_OCTO << RESET << "\n";
	std::cout << HACKER << std::setw(12) << "" << "Taskmaster" << RESET << "\n" << std::endl;

	if (argc != 1)
	{
		std::cerr << "Usage: " << argv[0] << std::endl;
		return (TM_FAILURE);
	}

	Logger::init();
	Logger::setDebug(true);

	if (check_pid_file() == TM_FAILURE)
	{
		Logger::error("Another instance is already running!");
		return (TM_FAILURE);
	}

	if (create_pid_file() == TM_FAILURE)
	{
		return (TM_FAILURE);
	}

	// setup_signal(SIGINT, SIG_IGN);
	// setup_signal(SIGQUIT, SIG_IGN);
	// setup_signal(SIGTERM, SIG_IGN);

	setup_signal(SIGINT, interruptHandler);

	// Taskmaster master(envp);

	// master.addChild(argv + 1);
	// master.start();
	
	try {
		start_server();
	} catch (const std::exception& e) {
		Logger::error(e.what());
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}
