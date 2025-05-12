/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:49 by mgama             #+#    #+#             */
/*   Updated: 2025/05/12 21:42:16 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signal.hpp"
#include "utils/utils.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

Taskmaster::Taskmaster(const std::string& config_file): _config_file(config_file)
{
	this->running = true;

	this->pid = getpid();
}

Taskmaster::Taskmaster(void): Taskmaster("")
{
}

Taskmaster::~Taskmaster(void)
{
	for(const auto& process : this->_unic_processes)
	{
		delete process;
	}

	setup_signal(SIGUSR2, SIG_IGN);
}

int
Taskmaster::addChild(char* const* exec, struct tm_process_config& config)
{
	for (int i = 0; i < config.numprocs; ++i)
	{
		Process* new_child = new Process(exec, std::string(exec[0]) + "_" + std::to_string(i), config, this->pid);
		new_child->setGroupId(i);
		this->_unic_processes.push_back(new_child);
	}
	return (TM_SUCCESS);
}

int
Taskmaster::cycle(void) const
{
	for(const auto& process : this->_unic_processes)
	{
		(void)process->monitor();
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			(void)process->monitor();
		}
	}

	return (TM_SUCCESS);
}

int
Taskmaster::start(void) const
{
	for(const auto& process : this->_unic_processes)
	{
		(void)process->start();
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			(void)process->start();
		}
	}

	return (TM_SUCCESS);
}

int
Taskmaster::restart(void) const
{
	for(const auto& process : this->_unic_processes)
	{
		(void)process->restart();
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			(void)process->restart();
		}
	}
	
	return (TM_SUCCESS);
}

int
Taskmaster::stop(void) const
{
	for(const auto& process : this->_unic_processes)
	{
		(void)process->stop();
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			(void)process->stop();
		}
	}

	return (TM_SUCCESS);
}

int
Taskmaster::signal(int sig) const
{
	for(const auto& process : this->_unic_processes)
	{
		(void)process->signal(sig);
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			(void)process->signal(sig);
		}
	}

	return (TM_SUCCESS);
}

int
Taskmaster::kill(void) const
{
	for(const auto& process : this->_unic_processes)
	{
		(void)process->kill();
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			(void)process->kill();
		}
	}

	return (TM_SUCCESS);
}

void
Taskmaster::reopenStds(void) const
{
	for(const auto& process : this->_unic_processes)
	{
		(void)process->reopenStds();
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			(void)process->reopenStds();
		}
	}

	Logger::reopenFileLogging();
}

bool
Taskmaster::allStopped() const
{
	for(const auto& process : this->_unic_processes)
	{
		if (false == process->stopped() && false == process->exited() && false == process->fatal())
			return (false);
	}

	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			if (false == process->stopped() && false == process->exited() && false == process->fatal())
				return (false);
		}
	}

	return (true);
}

size_t
Taskmaster::getNumProcesses(void) const
{
	return (this->_unic_processes.size());
}

const std::vector<Process*>&
Taskmaster::all(void) const
{
	return (this->_unic_processes);
}

Process*
Taskmaster::find(const std::string& progname) const
{
	for (const auto& process : this->_unic_processes)
	{
		if (process->getProgramName() == progname)
			return (process);
	}
	return (nullptr);
}

Process*
Taskmaster::get(uint16_t uid) const
{
	for (const auto& process : this->_unic_processes)
	{
		if (*process == uid)
			return (process);
	}
	return (nullptr);
}

std::string
Taskmaster::getDetailedStatus(void) const
{
	std::ostringstream oss;
	oss << "{\n";
	oss << "  PID: " << this->pid << ";\n";
	oss << "  Processes: {\n";
	for (const auto& process : this->_unic_processes)
	{
		oss << "    - Name: " << process->getProgramName() << " (" << process->getUid() << ")" << ";\n";
		oss << "      PID: " << process->getPid() << ";\n";
		oss << "      State: " << Process::getStateName(process->getState()) << ";\n";
		oss << "      Signal: " << process->getSignal() << ";\n";
		oss << "      Exit code: " << process->getExitCode() << ";\n";
		oss << "      Uptime: " << format_duration(process->uptime()) << ";\n";
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

std::string
Taskmaster::getProcsStatus(void) const
{
	std::ostringstream oss;
	for (const auto& process : this->_unic_processes)
	{
		oss << process->getStatus();
	}
	return oss.str();
}