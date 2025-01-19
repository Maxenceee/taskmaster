/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:49 by mgama             #+#    #+#             */
/*   Updated: 2025/01/19 14:21:25 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

static void interruptHandler(int sig_int) {
	(void)sig_int;
	std::cout << "\b\b"; // rm ^C from tty
	Logger::print("Signal received: " + std::string(strsignal(sig_int)), B_GREEN);
	Taskmaster::should_stop = true;
}

Taskmaster::Taskmaster(char* const* envp)
{
	this->exit = false;

	this->envp = envp;

	this->pid = getpid();
}

Taskmaster::~Taskmaster(void)
{
}

int	Taskmaster::addChild(char* const* exec)
{
	int	std_out_fd = open("child_stdout.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (std_out_fd == -1) {
		perror("Failed to open log file");
		return (1);
	}

	tm_process_config config = {
		.auto_restart = true
	};

	Process*	new_child = new Process(exec, -1, std_out_fd, -1, config);

	this->_processes.push_back(new_child);
	return (0);
}

int	Taskmaster::launch(void)
{
	for(const auto& process : this->_processes)
	{
		if (process->spawn(this->envp))
		{
			std::cout << "Could not spawn child" << std::endl;
		}
	}
	return (0);
}

bool stdinHasData()
{
	// using a timeout of 0 so we aren't waiting:
	struct timespec timeout{ 0l, 0l };

	// create a file descriptor set
	fd_set fds{};

	// initialize the fd_set to 0
	FD_ZERO(&fds);
	// set the fd_set to target file descriptor 0 (STDIN)
	FD_SET(STDIN_FILENO, &fds);

	// pselect the number of file descriptors that are ready, since
	//  we're only passing in 1 file descriptor, it will return either
	//  a 0 if STDIN isn't ready, or a 1 if it is.
	return pselect(0 + 1, &fds, nullptr, nullptr, &timeout, nullptr) == 1;
}

int	Taskmaster::start(void)
{
	Logger::print("taskmasterd started with pid "+std::to_string(this->pid));

	signal(SIGINT, interruptHandler);
	signal(SIGQUIT, interruptHandler);
	signal(SIGTERM, interruptHandler);

	// signal(SIGCHLD, interruptHandler);

	signal(SIGPIPE, SIG_IGN);

	this->launch();

	bool	handling_stop = false;

	std::string input;
	do
	{
		if (!this->should_stop && stdinHasData())
		{
			std::cin >> input;
			if ("rs" == input) {
				for(const auto& process : this->_processes)
				{
					if (process->exited())
					{
						if (process->spawn(this->envp))
						{
							std::cout << "Could not spawn child" << std::endl;
						}
					}
				}
			}
		}

		bool all_stopped = true;
		int s;
		for(const auto& process : this->_processes)
		{
			if (!(s = process->monitor()))
			{
				all_stopped = false;
			}
			std::cout << "Child " << process->getPid() << " monitor status: " << s << std::endl;
		}

		if (this->should_stop && !handling_stop)
		{
			for(const auto& process : this->_processes)
			{
				std::cout << "Stopping child " << process->getPid() << std::endl;
				if (process->stop())
					perror("Could not stop");
			}
			handling_stop = true;
		}

		if (this->should_stop && handling_stop && all_stopped)
			break;
		sleep(1);
	} while (false == this->exit);
	
	return (0);
}