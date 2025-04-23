/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/04/23 23:41:50 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "spawn.hpp"
#include "signal.hpp"
#include "pid.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"
#include "server/UnixSocketServer.hpp"
#include "daemon/daemon.hpp"

bool	Taskmaster::running = false;

void
interruptHandler(int sig_int)
{
	Logger::syn("\b\b"); // rm ^C from tty
	Logger::print("Signal received: " + std::string(strsignal(sig_int)), B_GREEN);
	if (Taskmaster::running == false)
	{
		Logger::info(TM_PROJECTD " is already stopping");
	}
	Taskmaster::running = false;
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

	Logger::print("Daemon started with pid: " + std::to_string(getpid()));

	tm_process_config config(
		1,
		false,
		TM_CONF_AUTORESTART_UNEXPECTED,
		{0, 4},
		TM_S_TERM,
		5,
		3,
		10
	);

	(void)master.addChild(argv + 1, config);
	if (argv[2] != nullptr)
	{
		tm_process_config config(
			1,
			false,
			TM_CONF_AUTORESTART_UNEXPECTED,
			{0, 4},
			TM_S_TERM,
			3,
			3,
			5
		);

		(void)master.addChild(argv + 2, config);
	}

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
	(void)master.stop();
	(void)server.shutdown();

	const auto current_time = std::chrono::system_clock::now();

	while (!master.allStopped())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		(void)master.cycle();
	}

	(void)server.stop();

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

	Logger::enableFileLogging(TM_MAIN_LOG_DIR TM_PROJECTD ".out.log", TM_MAIN_LOG_DIR TM_PROJECTD ".err.log");
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
