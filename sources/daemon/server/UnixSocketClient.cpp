/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClient.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:40:23 by mgama             #+#    #+#             */
/*   Updated: 2025/11/10 19:31:08 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

int
UnixSocketServer::Client::recv(void)
{
	char buff[TM_RECV_SIZE];

	ssize_t ret = ::recv(this->fd, buff, TM_RECV_SIZE, 0);
	if (ret == -1)
	{
		Logger::perror("server error: an error occurred while receiving data from the client");
		return (TM_FAILURE);
	}
	else if (ret == 0)
	{
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (this->parse(buff));
}

int
UnixSocketServer::Client::send(const std::string& msg)
{
	return (this->send(msg.c_str()));
}

int
UnixSocketServer::Client::send(const char* msg)
{
	return (this->send(msg, strlen(msg)));
}

int
UnixSocketServer::Client::send(const char* msg, size_t len)
{
	if (::send(this->fd, msg, len, 0) == -1)
	{
		Logger::perror("client error: send failed");
		return (TM_FAILURE);
	}
	return (TM_SUCCESS);
}

int
UnixSocketServer::Client::getFd(void) const
{
	return (this->fd);
}

int
UnixSocketServer::Client::done(void)
{
	if (false == this->input_received)
	{
		return (TM_POLL_CLIENT_OK);
	}

	size_t working = 0;
	for (auto& h : this->handlers)
	{
		if (this->_work(h) == TM_POLL_CLIENT_OK)
		{
			working++;
		}
	}

	if (working == 0)
	{
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_work(struct tm_pollclient_process_handler& ps)
{
	if (ps.done || ps.requested_state == -1)
	{
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		Logger::error("The process could not be found");
		ps.done = true;
		return (TM_POLL_CLIENT_ERROR);
	}
	if (p->reachedDesiredState())
	{
		if (ps.success_message != nullptr)
		{
			(void)this->send(p->getProcessName());
			(void)this->send(": ");
			(void)this->send(ps.success_message);
			(void)this->send(TM_CRLF);
		}
		ps.done = true;
		return (TM_POLL_CLIENT_DISCONNECT);	
	}

	if (p->getDesiredState() != ps.requested_state)
	{
		ps.requested_state = p->getDesiredState();
		(void)this->send("Request interrupted by another request");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_OK);
	}

	if (p->getState() == TM_P_FATAL || p->getState() == TM_P_UNKNOWN)
	{
		(void)this->send("The process is in a fatal state");
		(void)this->send(TM_CRLF);
		ps.done = true;
		return (TM_POLL_CLIENT_ERROR);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::parse(const char* buff)
{
	size_t pos = 0;
	std::string buffer(buff);

	if (this->input_received)
	{
		Logger::warning("A process is already running");
		return (TM_POLL_CLIENT_OK);
	}

	while ((pos = buffer.find(TM_CRLF)) != std::string::npos)
	{
		std::string line = buffer.substr(0, pos);
		buffer.erase(0, pos + 2);

		if (line.empty())
		{
			this->input_received = true;
			break;
		}

		this->input.push_back(line);
	}

	if (this->input_received)
	{
		this->exec();
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::exec(void)
{
	if (this->input.empty())
	{
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	for (const auto& line : this->input)
	{
		if (line.empty())
			break;

		if (line.rfind("Name:", 0) == 0)
		{
			this->name = line.substr(5);
			this->name.erase(0, this->name.find_first_not_of(" \t"));
		}
		else if (line.rfind("Args:", 0) == 0)
		{
			std::string argline = line.substr(5);
			std::istringstream iss(argline);
			std::string token;
			while (iss >> token)
			{
				this->args.push_back(token);
			}
		}
		else if (line.rfind("Opts:", 0) == 0)
		{
			std::string optline = line.substr(5);
			std::istringstream iss(optline);
			std::string token;
			while (iss >> token)
			{
				this->opts.push_back(token);
			}
		}
	}

	if (this->name.empty())
	{
		(void)this->send("Invalid command");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	auto itg = this->general_command_map.find(this->name);
	if (itg != this->general_command_map.end())
	{
		return ((this->*(itg->second))());
	}

	auto itp = this->process_command_map.find(this->name);
	if (itp != this->process_command_map.end())
	{
		if (this->args.empty())
		{
			(void)this->send("Invalid usage");
			(void)this->send(TM_CRLF);
			return (TM_POLL_CLIENT_ERROR);
		}

		auto ps = this->_find_processes(this->args);
		if (ps != TM_POLL_CLIENT_OK)
		{
			return (ps);
		}

		for (auto& h : this->handlers)
		{
			(void)(this->*(itp->second))(h);
		}

		return (TM_POLL_CLIENT_OK);
	}

	if (name == "internal")
	{
		if (this->args[0] == "processes")
		{
			if (this->opts[0] == "procs")
			{
				auto g = this->_master.getGroups();
				for (const auto& group : g)
				{
					(void)this->send(group->getName());
					(void)this->send(TM_CRLF);
					for (const auto& process : group->getReplicas())
					{
						(void)this->send(process->getProcessName());
						(void)this->send(TM_CRLF);
					}
				}
			}
			else if (this->opts[0] == "avail")
			{
				auto a = this->_master.getProgramsConf();
				for (const auto& p : a)
				{
					(void)this->send(p.name);
					(void)this->send(TM_CRLF);
				}
			}
		}
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	(void)this->send("Invalid command");
	(void)this->send(TM_CRLF);
	return (TM_POLL_CLIENT_ERROR);
}
