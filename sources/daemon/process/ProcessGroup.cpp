/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGroup.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 19:54:32 by mgama             #+#    #+#             */
/*   Updated: 2025/11/10 19:14:46 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "utils/utils.hpp"
#include "logger/Logger.hpp"

ProcessGroup::ProcessGroup(const tm_Config::Program &config): gid(u_uint16()), _config(config)
{
	for (int i = 0; i < config.numprocs; i++)
	{
		this->enque();
	}
}

ProcessGroup::~ProcessGroup(void)
{
	for (const auto& p : this->_replicas)
	{
		delete p;
	}
	for (const auto& p : this->_transitioning)
	{
		delete p;
	}
}

size_t
ProcessGroup::enque(void)
{
	auto newId = this->_replicas.size();

	auto p = new Process(this->_config, this->gid, this->_config.name, newId);
	Logger::debug("Enqueuing new process " + p->getProcessName() + " in group " + this->_config.name);
	this->_replicas.push_back(p);
	return (this->_replicas.size());
}

size_t
ProcessGroup::deque(void)
{
	auto last = this->_replicas.back();
	Logger::debug("Dequeuing process " + last->getProcessName() + " from group " + this->_config.name);
	last->markAsDead();
	this->_transitioning.push_back(last);
	this->_replicas.pop_back();
	return (this->_replicas.size());
}

void
ProcessGroup::monitor(void)
{
	for (const auto& process : this->_replicas)
	{
		(void)process->monitor();
	}

	auto it = this->_transitioning.begin();
	while (it != this->_transitioning.end())
	{
		(void)(*it)->monitor();
		if ((*it)->isDead())
		{
			delete *it;
			it = this->_transitioning.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void
ProcessGroup::update(tm_Config::Program &new_conf)
{
	bool shouldrestartprocs = false;
	const uint16_t old_numprocs = _config.numprocs;
    const uint16_t new_numprocs = new_conf.numprocs;

	if (
		this->_config.raw_command != new_conf.raw_command
		|| this->_config.environment != new_conf.environment
		|| this->_config.directory != new_conf.directory
		|| this->_config.umask != new_conf.umask
	)
	{
		if (new_conf.numprocs > 0)
		{
			shouldrestartprocs = true;
		}
	}

	for (const auto& p : this->_replicas)
	{
		p->update(new_conf);
	}

	if (this->_config.stdout_logfile != new_conf.stdout_logfile || this->_config.stderr_logfile != new_conf.stderr_logfile)
	{
		for (const auto& p : this->_replicas)
		{
			p->reopenStds();
		}
	}

	this->_config = new_conf;

	if (shouldrestartprocs)
	{
		Logger::debug("ProcessGroup " + this->_config.name + " should be restarted due to config changes");
		for (const auto& p : this->_replicas)
		{
			if (!(p->stopped() || p->exited() || p->fatal()))
			{	
				(void)p->restart();
			}
		}
	}

	if (old_numprocs < new_numprocs)
	{
		Logger::debug("ProcessGroup " + this->_config.name + " increasing number of processes from " + std::to_string(old_numprocs) + " to " + std::to_string(new_numprocs));
		for (int i = old_numprocs; i < new_numprocs; i++)
		{
			(void)this->enque();
		}
	}
	else if (old_numprocs > new_numprocs)
	{
		Logger::debug("ProcessGroup " + this->_config.name + " decreasing number of processes from " + std::to_string(old_numprocs) + " to " + std::to_string(new_numprocs));
		for (int i = old_numprocs; i > new_numprocs; i--)
		{
			(void)this->deque();
		}
	}
}

void
ProcessGroup::remove(void)
{
	for (size_t i = 0; i < this->_replicas.size(); i++)
	{
		(void)this->deque();
	}
}

bool
ProcessGroup::safeToRemove(void) const
{
	if (!this->_replicas.empty())
	{
		return (false);
	}

	for (const auto& p : this->_transitioning)
	{
		if (p->isDead() == false)
			return (false);
	}
	return (true);
}

const std::vector<Process*>&
ProcessGroup::getReplicas(void) const
{
	return (this->_replicas);
}

const std::string&
ProcessGroup::getName(void) const
{
	return (this->_config.name);
}

std::ostream&
operator<<(std::ostream& os, const ProcessGroup& group)
{
	for (const auto* p : group.getReplicas())
	{
		os << *p;
	}
	for (const auto* p : group._transitioning)
	{
		os << *p;
	}
	return os;
}

std::ostream&
ProcessGroup::operator<<(std::ostream& os) const
{
	for (const auto* p : this->_replicas)
	{
		os << *p;
	}
	return os;
}

bool
ProcessGroup::operator==(uint16_t other) const
{
	return (this->gid == other);
}

bool
ProcessGroup::operator==(const std::string& other) const
{
	return (this->_config.name == other);
}
