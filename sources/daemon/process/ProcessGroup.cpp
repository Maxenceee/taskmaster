/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGroup.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 19:54:32 by mgama             #+#    #+#             */
/*   Updated: 2025/05/13 19:56:15 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "utils/utils.hpp"

ProcessGroup::ProcessGroup(tm_Config::Program &config): gid(u_uint16()), _config(config)
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

void
ProcessGroup::enque(void)
{
	auto newId = this->_replicas.size();

	auto p = new Process(this->_config, this->gid, this->_config.name, newId);
	this->_replicas.push_back(p);
}

void
ProcessGroup::deque(void)
{
	auto last = this->_replicas.back();
	last->markAsDead();
	this->_transitioning.push_back(last);
	this->_replicas.pop_back();
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
	}
}

void
ProcessGroup::update(tm_Config::Program &new_conf)
{
	bool shouldrestartprocs = false;

	if (
		this->_config.command != new_conf.command
		|| this->_config.environment != new_conf.environment
		|| this->_config.directory != new_conf.directory
		|| this->_config.umask != new_conf.umask
	)
	{
		shouldrestartprocs = true;
	}
	// TODO: update process_name

	if (this->_config.stdout_logfile != new_conf.stdout_logfile || this->_config.stderr_logfile != new_conf.stderr_logfile)
	{
		for (const auto& p : this->_replicas)
		{
			p->reopenStds();
		}
	}
	
	uint16_t old_numprocs = this->_config.numprocs; 
	this->_config = new_conf;

	if (shouldrestartprocs)
	{
		for (const auto& p : this->_replicas)
		{
			(void)p->restart();
		}
	}

	if (old_numprocs < new_conf.numprocs)
	{
		for (int i = new_conf.numprocs - old_numprocs; i < new_conf.numprocs; i++)
		{
			this->enque();
		}
	}
	else if (old_numprocs > new_conf.numprocs)
	{
		for (int i = old_numprocs - new_conf.numprocs; i < new_conf.numprocs; i++)
		{
			this->deque();
		}
	}
}

const std::vector<Process*>&
ProcessGroup::getReplicas(void) const
{
	return (this->_replicas);
}

std::ostream&
operator<<(std::ostream& os, const ProcessGroup& group)
{
	for (const auto* p : group.getReplicas())
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