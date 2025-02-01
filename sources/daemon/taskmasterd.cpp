/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/02/01 01:01:31 by mgama            ###   ########.fr       */
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

// bool
// is_process_running(pid_t pid)
// {
// 	return (kill(pid, 0) == 0 || errno != ESRCH);
// }

// int
// create_pid_file(void)
// {
// 	int pid_fd = open(TM_PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
// 	if (pid_fd == -1) {
// 		perror("open");
// 		return (TM_FAILURE);
// 	}

// 	if (flock(pid_fd, LOCK_EX | LOCK_NB) == -1) {
// 		std::cerr << "Another instance is already running!" << std::endl;
// 		return (TM_FAILURE);
// 	}

// 	// Écrire le PID dans le fichier
// 	char pid_str[10];
// 	snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
// 	write(pid_fd, pid_str, strlen(pid_str));

// 	return (TM_SUCCESS);
// }

int
main(int argc, char* const* argv, char* const* envp)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <command> [args...]" << std::endl;
		return (1);
	}

	// if (create_pid_file() == TM_FAILURE)
	// {
	// 	return (1);
	// }

	// setup_signal(SIGINT, SIG_IGN);
	// setup_signal(SIGQUIT, SIG_IGN);
	// setup_signal(SIGTERM, SIG_IGN);

	setup_signal(SIGINT, interruptHandler);

	Logger::init();
	Logger::setDebug(true);

	// Taskmaster master(envp);

	// master.addChild(argv + 1);
	// master.start();

	UnixSocketServer server(TM_SOCKET_PATH);
	server.listen();
	
	do
	{
		if (server.poll())
		{
			break;
		}
	} while (running);
	
}
