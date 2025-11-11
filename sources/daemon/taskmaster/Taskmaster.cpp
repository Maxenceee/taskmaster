/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:49 by mgama             #+#    #+#             */
/*   Updated: 2025/11/11 13:55:29 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signal.hpp"
#include "utils/utils.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

inline static void
_ensure_child_run_dir_exists(void)
{
	struct stat st;
	if (stat(TM_RUN_DIR, &st) == -1)
	{
		switch (errno)
		{
		case ENOENT:
			if (mkdir(TM_RUN_DIR, 0755) != -1)
			{
				break;
			}
			[[fallthrough]]; // Indicate to the compiler that we want to fallback to the next case
		case EACCES:
			Logger::warning("Permission denied for " TM_RUN_DIR);
			[[fallthrough]];
		default:
			throw std::runtime_error("Invalid run directory path: " TM_RUN_DIR);
		}
	}
	else if (!S_ISDIR(st.st_mode))
	{
		throw std::runtime_error("Invalid run directory path: " TM_RUN_DIR);
	}
	else if (access(TM_RUN_DIR, W_OK) == -1)
	{
		throw std::runtime_error("Invalid run directory path: " TM_RUN_DIR);
	}
}

Taskmaster::Taskmaster(const std::string& config_file): _config_file(config_file)
{
	this->reload = false;
	this->running = true;

	this->pid = getpid();

	_ensure_child_run_dir_exists();
}

Taskmaster::Taskmaster(void): Taskmaster("") {}

Taskmaster::~Taskmaster(void)
{
	for(const auto& group : this->_processes)
	{
		delete group;
	}
	for (const auto& group : this->_transitioning)
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
Taskmaster::stop(void)
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
Taskmaster::add(const std::string& progname)
{
	auto it = std::find_if(
		this->_active_config.programs.begin(),
		this->_active_config.programs.end(),
		[&progname](const tm_Config::Program& prog) {
			return prog.name == progname;
		}
	);
	if (it != this->_active_config.programs.end())
	{
		auto group = std::find_if(
			this->_processes.begin(),
			this->_processes.end(),
			[&progname](const ProcessGroup* group) {
				return *group == progname;
			}
		);
		if (group == this->_processes.end())
		{
			auto newp = new ProcessGroup(*it);
			this->_processes.push_back(newp);
			return (TM_SUCCESS);
		}
	}
	return (TM_FAILURE);
}

int
Taskmaster::remove(const std::string& progname)
{
	auto it = std::find_if(
		this->_processes.begin(),
		this->_processes.end(),
		[&progname](const ProcessGroup* group) {
			return *group == progname;
		}
	);
	if (it != this->_processes.end())
	{
		this->_remove(*it);
		return (TM_SUCCESS);
	}
	return (TM_FAILURE);
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

ProcessGroup*
Taskmaster::findGroup(const std::string& progname) const
{
	for (auto* group : this->_processes)
	{
		if (*group == progname)
			return (group);
	}
	return (nullptr);
}

Process*
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

Process*
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

const std::vector<ProcessGroup*>&
Taskmaster::getGroups(void) const
{
	return this->_processes;
}

std::string
Taskmaster::getProcsStatus(void) const
{
	std::ostringstream oss;
	if (this->_processes.size() == 0 && this->_transitioning.size() == 0)
	{
		oss << "No processes running\n";
		return oss.str();
	}

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

const tm_Config::Daemon&
Taskmaster::getDaemonConf(void) const
{
	return (this->_active_config.daemon);
}

const std::vector<tm_Config::Program>&
Taskmaster::getProgramsConf(void) const
{
	return (this->_active_config.programs);
}
