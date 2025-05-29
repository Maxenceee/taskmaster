/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:49 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 15:13:18 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signal.hpp"
#include "utils/utils.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

Taskmaster::Taskmaster(const std::string& config_file): _config_file(config_file)
{
	this->reload = false;
	this->running = true;

	this->pid = getpid();
}

Taskmaster::Taskmaster(void): Taskmaster("")
{
}

Taskmaster::~Taskmaster(void)
{
	for(const auto& group : this->_processes)
	{
		delete group;
	}
}

int
Taskmaster::cycle(void)
{
	for (const auto group : this->_processes)
	{
		group->monitor();
	}

	for (auto it = this->_transitioning.begin(); it != this->_transitioning.end(); )
	{
		auto group = *it;
		group->monitor();
		if (group->safeToRemove())
		{
			it = this->_transitioning.erase(it);
			delete group;
		}
		else
		{
			++it;
		}
	}

	return (TM_SUCCESS);
}

int
Taskmaster::start(void) const
{
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

const std::vector<Process*>
Taskmaster::all(void) const
{
	std::vector<Process*> res;
	for (const auto& group : this->_processes)
	{
		for (const auto& process : group->getReplicas())
		{
			res.push_back(process);
		}
	}
	return (res);
}

Process* const
Taskmaster::find(const std::string& progname) const
{
	for (const auto* group : this->_processes)
	{
		for (auto* process : group->getReplicas())
		{
			if (*process == progname)
				return (process);
		}
	}
	return (nullptr);
}

Process* const
Taskmaster::get(uint32_t uid) const
{
	uint16_t gid = TM_P_GID(uid);
	uint16_t pid = TM_P_PID(uid);

	for (const auto* group : this->_processes)
	{
		if (*group == gid)
		{
			for (auto* process : group->getReplicas())
			{
				if (*process == pid)
					return (process);
			}
		}
	}

	return (nullptr);
}

std::string
Taskmaster::getProcsStatus(void) const
{
	std::ostringstream oss;
	for (const auto* group : this->_processes)
	{
		oss << *group;
	}
	for (const auto* group : this->_transitioning)
	{
		oss << *group;
	}
	return oss.str();
}

std::string
Taskmaster::getAvailableProcs(void) const
{
	std::unordered_set<std::string> in_use;
	std::ostringstream oss;

	for (const auto& group : this->_processes)
	{
		in_use.insert(group->getName());
	}

	for (const auto& program : this->_read_config.programs)
	{
		bool is_in_use = in_use.find(program.name) != in_use.end();

		oss << std::setw(30) << std::left << program.name << " ";
		oss << std::setw(10) << std::left << (is_in_use ? "in use" : "avail") << " ";
		oss << std::setw(10) << std::left << (program.autostart ? "auto" : "manual") << " ";
		oss << "\n";
	}

	return (oss.str());
}

const tm_Config::UnixServer&
Taskmaster::getServerConf(void) const
{
	return (this->_active_config.server);
}
