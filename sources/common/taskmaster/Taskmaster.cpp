/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:49 by mgama             #+#    #+#             */
/*   Updated: 2025/04/19 10:50:52 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signal.hpp"
#include "utils/utils.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

Taskmaster::Taskmaster(char* const* envp)
{
	this->exit = false;

	this->envp = envp;

	this->pid = getpid();
}

Taskmaster::~Taskmaster(void)
{
	for(const auto& process : this->_processes)
	{
		delete process;
	}
}

int
Taskmaster::addChild(char* const* exec)
{
	// Temp output file
	int	std_out_fd = open("child_stdout.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (std_out_fd == -1) {
		Logger::perror("open");
		return (TM_FAILURE);
	}

	tm_process_config config(true, TM_CONF_AUTORESTART_UNEXPECTED, {0, 4}, TERM, 1, 3);

	Process* new_child = new Process(exec, this->envp, this->pid, -1, std_out_fd, -1, config);

	this->_processes.push_back(new_child);
	return (TM_SUCCESS);
}

// bool
// stdinHasData()
// {
// 	// using a timeout of 0 so we aren't waiting:
// 	struct timespec timeout{ 0l, 0l };

// 	// create a file descriptor set
// 	fd_set fds{};

// 	// initialize the fd_set to 0
// 	FD_ZERO(&fds);
// 	// set the fd_set to target file descriptor 0 (STDIN)
// 	FD_SET(STDIN_FILENO, &fds);

// 	// pselect the number of file descriptors that are ready, since
// 	//  we're only passing in 1 file descriptor, it will return either
// 	//  a 0 if STDIN isn't ready, or a 1 if it is.
// 	return pselect(0 + 1, &fds, nullptr, nullptr, &timeout, nullptr) == 1;
// }

int
Taskmaster::cycle(void)
{
	for(const auto& process : this->_processes)
	{
		(void)process->monitor();
		// switch (process->monitor())
		// {
		// 	case 0:
		// 		break;
		// 	case 1:
		// 		Logger::info("Child " + std::to_string(process->getPid()) + " exited");
		// 		if (process->shouldRestart())
		// 		{
		// 			if (process->spawn(this->envp))
		// 			{
		// 				std::cout << "Could not spawn child" << std::endl;
		// 			}
		// 		}
		// 		break;
		// }
		// std::cout << "Child " << process->getPid() << " monitor status: " << s << std::endl;
	}

	return (TM_SUCCESS);
}

int
Taskmaster::stop(void)
{
	for(const auto& process : this->_processes)
	{
		(void)process->stop();
	}
	return (TM_SUCCESS);
}

int
Taskmaster::kill(void)
{
	for(const auto& process : this->_processes)
	{
		std::string sig = getSignalName(process->getStopSignal());
		Logger::info("StopSignal " + sig + " failed to stop child in 10 seconds, resorting to SIGKILL");
		(void)process->kill();
	}
	return (TM_SUCCESS);
}

bool
Taskmaster::allStopped() const
{
	for(const auto& process : this->_processes)
	{
		if (false == process->stopped() && false == process->exited() && false == process->fatal())
			return (false);
	}
	return (true);
}
