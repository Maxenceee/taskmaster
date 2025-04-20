/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/04/20 18:04:11 by mgama            ###   ########.fr       */
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
bool	Taskmaster::reload = false;

static int g_pid_fd = -1;

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

	tm_pid = getpid();

	pid_fd = open(TM_PID_FILE, O_WRONLY | O_CREAT, TM_DEFAULT_FILE_MODE);
	if (pid_fd == -1)
	{
		Logger::perror("open");
		return (TM_FAILURE);
	}

	if (flock(pid_fd, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            Logger::error("Another instance is already running!");
        } else {
            Logger::perror("flock");
        }
        close(pid_fd);
        return (TM_FAILURE);
    }

	ftruncate(pid_fd, 0);
	dprintf(pid_fd, "%d\n", tm_pid);

	g_pid_fd = pid_fd;

	return (TM_SUCCESS);
}

void
remove_pid_file(void)
{
	if (g_pid_fd != -1) {
		flock(g_pid_fd, LOCK_UN);
        close(g_pid_fd);
        g_pid_fd = -1;
    }

	if (unlink(TM_PID_FILE) == -1)
	{
		Logger::perror("unlink");
	}
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

	std::cout << "Starting with " << master.getNumProcesses() << " processes" << std::endl;

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

	while (!master.allStopped())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		(void)master.cycle();
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

	if (create_pid_file() == TM_FAILURE)
	{
		return (TM_FAILURE);
	}

	setup_signal(SIGINT, SIG_IGN);
	setup_signal(SIGQUIT, SIG_IGN);
	setup_signal(SIGTERM, SIG_IGN);

	setup_signal(SIGPIPE, SIG_IGN);

	Logger::print("Daemon started with pid: " + std::to_string(getpid()));

	try
	{
		do
		{
			Taskmaster::reload = false;
			start_main_loop(argv, envp);
		} while (Taskmaster::reload);
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
