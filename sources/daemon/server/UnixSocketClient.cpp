/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClient.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:40:23 by mgama             #+#    #+#             */
/*   Updated: 2025/04/23 11:08:48 by mgama            ###   ########.fr       */
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
		Logger::debug("Connection closed by the client");
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
	if (::send(this->fd, msg, strlen(msg), 0) == -1)
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
		std::stringstream ss;
		ss << "Process " << p->getPid() << " (" << p->getProgramName() << ") is now " << Process::getStateName(p->getState()) << " [" << Process::getStateName(p->getDesiredState()) << "]" << " - " << format_duration(p->uptime()) << "\n";

		(void)this->send(ss.str());
		ps.done = true;
		return (TM_POLL_CLIENT_DISCONNECT);	
	}

	if (p->getDesiredState() != ps.requested_state)
	{
		ps.requested_state = p->getDesiredState();
		(void)this->send("Request interrupted by another request\n");
		return (TM_POLL_CLIENT_OK);
	}

	if (p->getState() == TM_P_FATAL || p->getState() == TM_P_UNKNOWN)
	{
		Logger::error("The process is in a fatal state");
		(void)this->send("The process is in a fatal state\n");
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
		(void)this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	const std::string& command = this->input[0];
	if (command.empty())
	{
		(void)this->send("Invalid command\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	auto itg = this->general_command_map.find(command);
	if (itg != this->general_command_map.end())
	{
		return ((this->*(itg->second))());
	}

	auto itp = this->process_command_map.find(command);
	if (itp != this->process_command_map.end())
	{
		if (this->input.size() < 2)
		{
			(void)this->send("Invalid usage\n");
			return (TM_POLL_CLIENT_ERROR);
		}

		std::vector<std::string> processes(this->input.begin() + 1, this->input.end());

		auto ps = this->_find_processes(processes);
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

	if (command == "internal")
	{
		std::cout << "Internal command " << this->input[1] << "\n";
		if (this->input.size() == 3 && this->input[1] == "processes" && this->input[2] == "avail")
		{
			auto a = this->_master.all();
			for (const auto& p : a)
			{
				(void)this->send(p->getProgramName());
				(void)this->send(TM_CRLF);
			}
			return (TM_POLL_CLIENT_DISCONNECT);
		}
	}

	(void)this->send("Invalid command\n");
	return (TM_POLL_CLIENT_ERROR);
}