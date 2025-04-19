/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/04/19 11:35:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "spawn.hpp"
#include "signal.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"
#include "server/UnixSocketServer.hpp"
#include "daemon/daemon.hpp"

bool	Taskmaster::running = false;

void
interruptHandler(int sig_int)
{
	Logger::cout("\b\b"); // rm ^C from tty
	Logger::print("Signal received: " + std::string(strsignal(sig_int)), B_GREEN);
	if (Taskmaster::running == false)
	{
		Logger::info(TM_PROJECTD " is already stopping");
	}
	Taskmaster::running = false;
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

void
remove_pid_file(void)
{
	if (unlink(TM_PID_FILE) == -1)
	{
		Logger::perror("unlink");
	}
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
start_main_loop(char* const* argv, char* const* envp)
{
	Taskmaster master(envp);

	UnixSocketServer server(TM_SOCKET_PATH, master);
	server.listen();

	setup_signal(SIGINT, interruptHandler);
	setup_signal(SIGQUIT, interruptHandler);
	setup_signal(SIGTERM, interruptHandler);

	(void)master.addChild(argv + 1);

	do
	{
		if (server.cycle())
		{
			break;
		}
		if (master.cycle())
		{
			break;
		}
	} while (Taskmaster::running);

	Logger::print(TM_PROJECTD " stopping, this can take a while...", B_GREEN);
	(void)server.stop();
	(void)master.stop();

	const auto current_time = std::chrono::system_clock::now();

	while (!master.allStopped() && std::chrono::system_clock::now() - current_time < std::chrono::seconds(10))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		(void)master.cycle();
	}

	if (!master.allStopped())
	{
		(void)master.kill();
	}

	Logger::print(TM_PROJECTD " stopped", B_GREEN);
}

int
main(int argc, char* const* argv, char* const* envp)
{
	Logger::printHeader();

	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <cmd>" << std::endl;
		return (TM_FAILURE);
	}

	// if (become_daemon(TM_NO_CHDIR) == TM_FAILURE)
	// {
	// 	Logger::error("Could not become daemon");
	// 	return (TM_FAILURE);
	// }

	Logger::init("Starting daemon");
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

	setup_signal(SIGINT, SIG_IGN);
	setup_signal(SIGQUIT, SIG_IGN);
	setup_signal(SIGTERM, SIG_IGN);

	setup_signal(SIGPIPE, SIG_IGN);

	try
	{
		start_main_loop(argv, envp);
	}
	catch (const std::exception& e)
	{
		Logger::error(e.what());
		remove_pid_file();
		return (TM_FAILURE);
	}
	remove_pid_file();

	return (TM_SUCCESS);
}
