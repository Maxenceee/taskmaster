/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:49 by mgama             #+#    #+#             */
/*   Updated: 2025/04/19 19:16:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signal.hpp"
#include "utils/utils.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

Taskmaster::Taskmaster(char* const* envp)
{
	this->running = true;

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

	tm_process_config config(
		1,
		false,
		TM_CONF_AUTORESTART_UNEXPECTED,
		{0, 4},
		TERM,
		5,
		3
	);

	Process* new_child = new Process(exec, this->envp, "child_key", config, this->pid);
	new_child->setStdOutFd(std_out_fd); // temp

	this->_processes.push_back(new_child);

	for (int i = 1; i < new_child->getNumProcs(); ++i)
	{
		Process* new_child = new Process(exec, this->envp, "child_key", config, this->pid);
		new_child->setStdOutFd(std_out_fd);  // temp
		new_child->setGroupId(i);
		this->_processes.push_back(new_child);
	}
	return (TM_SUCCESS);
}

int
Taskmaster::cycle(void)
{
	for(const auto& process : this->_processes)
	{
		(void)process->monitor();
	}

	return (TM_SUCCESS);
}

int
Taskmaster::start(void)
{
	for(const auto& process : this->_processes)
	{
		(void)process->start();
	}
	return (TM_SUCCESS);
}

int
Taskmaster::restart(void)
{
	for(const auto& process : this->_processes)
	{
		(void)process->restart();
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
Taskmaster::signal(int sig)
{
	for(const auto& process : this->_processes)
	{
		(void)process->signal(sig);
	}
	return (TM_SUCCESS);
}

int
Taskmaster::kill(void)
{
	for(const auto& process : this->_processes)
	{
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

size_t
Taskmaster::getNumProcesses(void) const
{
	return (this->_processes.size());
}

std::string
Taskmaster::getStatus(void) const
{
	std::ostringstream oss;
	oss << "{\n";
	oss << "  PID: " << this->pid << ";\n";
	oss << "  Processes: {\n";
	for (const auto& process : this->_processes)
	{
		oss << "    - Name: " << process->getProgramName() << ";\n";
		oss << "      PID: " << process->getPid() << ";\n";
		oss << "      State: " << process->getState() << ";\n";
		oss << "      Signal: " << process->getSignal() << ";\n";
		oss << "      Exit code: " << process->getExitCode() << ";\n";
		oss << "      Program: " << process->getExecName() << ";\n";
		oss << "      ProgramArguments: (" << "\n";
		for (char* const* arg = process->getExecArgs(); *arg != nullptr; ++arg)
		{
			oss << "        " << *arg << ";\n";
		}
		oss << "      );\n";
	}
	oss << "  };\n";
	oss << "  Running: " << (this->running ? "true" : "false") << ";\n";
	oss << "}\n";
	return oss.str();
}