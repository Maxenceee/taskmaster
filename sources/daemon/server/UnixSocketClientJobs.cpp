/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClientJobs.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:46:05 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 18:28:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

const std::unordered_map<std::string, UnixSocketServer::Client::ProcHandler> UnixSocketServer::Client::process_command_map = {
	{"add", &UnixSocketServer::Client::_add},
	{"clear", &UnixSocketServer::Client::_clear},
	{"pid", &UnixSocketServer::Client::_pid},
	{"remove", &UnixSocketServer::Client::_remove},
	{"restart", &UnixSocketServer::Client::_restart},
	{"start", &UnixSocketServer::Client::_start},
	{"stop", &UnixSocketServer::Client::_stop}
};

const std::unordered_map<std::string, UnixSocketServer::Client::GenHandler> UnixSocketServer::Client::general_command_map = {
	{"avail", &UnixSocketServer::Client::_avail},
	{"maintail", &UnixSocketServer::Client::_maintail},
	{"reload", &UnixSocketServer::Client::_reload},
	{"reread", &UnixSocketServer::Client::_reread},
	{"shutdown", &UnixSocketServer::Client::_shutdown},
	{"signal", &UnixSocketServer::Client::_signal},
	{"status", &UnixSocketServer::Client::_status},
	{"tail", &UnixSocketServer::Client::_tail},
	{"update", &UnixSocketServer::Client::_update},
	{"version", &UnixSocketServer::Client::_version}
};

int
UnixSocketServer::Client::_find_processes(const std::vector<std::string>& progs)
{
	if (progs.empty())
	{
		this->send("Invalid usage: No process specified\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	if (progs.size() == 1 && progs[0] == "all")
	{
		this->handlers.clear();
		for (const auto& p : this->_master.all())
		{
			this->handlers.push_back({p->getUid(), p->getState(), 0, false});
		}
		return (TM_POLL_CLIENT_OK);
	}

	for (const auto& prog : progs)
	{
		auto p = this->_master.find(prog);
		if (!p)
		{
			this->send("The process " + prog + " could not be found\n");
			continue;
		}

		this->handlers.push_back({p->getUid(), p->getState(), 0, false});
	}

	if (this->handlers.empty())
	{
		this->send("No process found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_add(struct tm_pollclient_process_handler& ps)
{
	(void)ps;
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_avail(void)
{
	if (this->input.size() > 1)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_clear(struct tm_pollclient_process_handler& ps)
{
	(void)ps;
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_maintail()
{
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_pid(struct tm_pollclient_process_handler& ps)
{
	(void)ps;
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_reload()
{
	if (this->input.size() > 1)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	this->send("Reloading the daemon\n");

	if (this->_master.restart())
	{
		this->send("Fail to reload the daemon\n");
	}

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_remove(struct tm_pollclient_process_handler& ps)
{
	(void)ps;
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_reread(void)
{
	if (this->input.size() > 1)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_restart(struct tm_pollclient_process_handler& ps)
{	
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		this->send("The process could not be found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	switch (p->getState())
	{
	case TM_P_STOPPING:
		this->send("The process is in transition state\n");
		return (TM_POLL_CLIENT_DISCONNECT);	
	case TM_P_BACKOFF:
	case TM_P_FATAL:
		this->send("The process is in a fatal state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_UNKNOWN:
		this->send("Process is in an unknown state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_RUNNING;

	if (p->restart())
	{
		this->send("Failed to restart the process\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_shutdown(void)
{
	if (this->input.size() > 1)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	this->send("Shutting down the daemon\n");
	Taskmaster::running = false;

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_signal(void)
{
	if (this->input.size() < 3)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	int signal = std::stoi(this->input[1]);

	for (auto it = this->input.begin() + 2; it != this->input.end(); ++it)
	{
		auto p = this->_master.find(*it);
		if (!p)
		{
			this->send("The process " + *it + " could not be found\n");
			continue;
		}

		if (p->getState() != TM_P_RUNNING)
		{
			this->send("The process is not running\n");
			continue;
		}

		this->send("Sending signal " + std::to_string(signal) + " to process " + *it + "\n");

		(void)p->signal(signal);
	}

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_start(struct tm_pollclient_process_handler& ps)
{
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		this->send("The process could not be found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	switch (p->getState())
	{
	case TM_P_RUNNING:
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_STARTING:
	case TM_P_STOPPING:
		this->send("The process is in transition state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_BACKOFF:
	case TM_P_FATAL:
		this->send("The process is in a fatal state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_UNKNOWN:
		this->send("Process is in an unknown state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_RUNNING;

	if (p->start())
	{
		this->send("Failed to start the process\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_status(void)
{
	if (this->input.size() == 1 || this->input[1] == "all")
	{
		this->send(this->_master.getStatus());
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	std::vector<std::string> processes(this->input.begin() + 1, this->input.end());

	for (auto it = this->input.begin() + 1; it != this->input.end(); ++it)
	{
		auto p = this->_master.find(*it);
		if (!p)
		{
			this->send("The process " + *it + " could not be found\n");
			continue;
		}

		this->send(p->getStatus());
	}
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_stop(struct tm_pollclient_process_handler& ps)
{
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		this->send("The process could not be found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	switch (p->getState())
	{
	case TM_P_STOPPING:
		this->send("The process is in transition state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_STOPPED:
	case TM_P_EXITED:
		this->send("The process is not running\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_FATAL:
	case TM_P_UNKNOWN:
		this->send("Process is in an unknown state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_EXITED;

	if (p->stop())
	{
		this->send("Failed to stop the process\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_tail(void)
{
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_update(void)
{
	if (this->input.size() > 1)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_version(void)
{
	if (this->input.size() > 1)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	this->send(TM_PROJECTD " version " TM_VERSION " - " TM_AUTHOR "\n");
	return (TM_POLL_CLIENT_DISCONNECT);
}