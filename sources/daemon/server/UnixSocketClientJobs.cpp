/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClientJobs.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:46:05 by mgama             #+#    #+#             */
/*   Updated: 2025/05/01 10:25:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

const std::unordered_map<std::string, UnixSocketServer::Client::ProcHandler> UnixSocketServer::Client::process_command_map = {
	{"add", &UnixSocketServer::Client::_add},
	{"clear", &UnixSocketServer::Client::_clear},
	{"remove", &UnixSocketServer::Client::_remove},
	{"restart", &UnixSocketServer::Client::_restart},
	{"start", &UnixSocketServer::Client::_start},
	{"stop", &UnixSocketServer::Client::_stop}
};

const std::unordered_map<std::string, UnixSocketServer::Client::GenHandler> UnixSocketServer::Client::general_command_map = {
	{"avail", &UnixSocketServer::Client::_avail},
	{"maintail", &UnixSocketServer::Client::_maintail},
	{"pid", &UnixSocketServer::Client::_pid},
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
		(void)this->send("Invalid usage: No process specified\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	if (progs.size() == 1 && progs[0] == "all")
	{
		this->handlers.clear();
		for (const auto& p : this->_master.all())
		{
			this->handlers.push_back({p->getUid(), p->getState(), -1, false});
		}
		return (TM_POLL_CLIENT_OK);
	}

	for (const auto& prog : progs)
	{
		auto p = this->_master.find(prog);
		if (!p)
		{
			(void)this->send("The process " + prog + " could not be found\n");
			continue;
		}

		this->handlers.push_back({p->getUid(), p->getState(), 0, false});
	}

	if (this->handlers.empty())
	{
		(void)this->send("No process found\n");
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
		(void)this->send("Invalid usage\n");
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
UnixSocketServer::Client::_maintail(void)
{
	auto ref = Logger::dump(TM_LOG_FILE_STDOUT);

	ref.seekg(0, std::ios::end);
	std::streamsize fileSize = ref.tellg();

	std::streamsize startPos = std::max(static_cast<std::streamsize>(0), fileSize - 1600);

	ref.seekg(startPos, std::ios::beg);

	char buf[TM_RECV_SIZE];
	std::streamsize bytesRead;
	while (ref)
	{
		ref.read(buf, TM_RECV_SIZE);
		bytesRead = ref.gcount();
		(void)this->send(buf, bytesRead);
	}
	
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_pid(void)
{
	if (this->input.size() > 2)
	{
		(void)this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	if (this->input.size() == 1)
	{
		(void)this->send("Daemon pid: " + std::to_string(getpid()) + "\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}
	
	if (this->input[1] == "all")
	{
		for (const auto& p : this->_master.all())
		{
			(void)this->send("Process " + p->getProgramName() + " pid: " + std::to_string(p->getPid()) + "\n");
		}
	}
	else
	{
		auto p = this->_master.find(this->input[1]);
		if (!p)
		{
			(void)this->send("The process could not be found\n");
			return (TM_POLL_CLIENT_DISCONNECT);
		}
		(void)this->send("Process " + p->getProgramName() + " pid: " + std::to_string(p->getPid()) + "\n");
	}
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_reload()
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	(void)this->send("Reloading the daemon\n");

	if (this->_master.restart())
	{
		(void)this->send("Fail to reload the daemon\n");
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
		(void)this->send("Invalid usage\n");
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
		(void)this->send("The process could not be found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	switch (p->getState())
	{
	case TM_P_STOPPING:
		(void)this->send("The process is in transition state\n");
		return (TM_POLL_CLIENT_DISCONNECT); 
	case TM_P_BACKOFF:
	case TM_P_FATAL:
		(void)this->send("The process is in a fatal state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_UNKNOWN:
		(void)this->send("Process is in an unknown state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_RUNNING;

	if (p->restart())
	{
		(void)this->send("Failed to restart the process\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_shutdown(void)
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	(void)this->send("Shutting down the daemon...\n");
	Taskmaster::running = false;

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_signal(void)
{
	if (this->input.size() < 3)
	{
		(void)this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	int signal;
	try
	{
		signal = std::stoi(this->input[1]);
	}
	catch(...)
	{
		(void)this->send("Invalid signal\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	if (signal < 1 || signal > NSIG)
	{
		(void)this->send("Invalid signal\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	for (auto it = this->input.begin() + 2; it != this->input.end(); ++it)
	{
		auto p = this->_master.find(*it);
		if (!p)
		{
			(void)this->send("The process " + *it + " could not be found\n");
			continue;
		}

		if (p->getState() != TM_P_RUNNING)
		{
			(void)this->send("The process is not running\n");
			continue;
		}

		(void)this->send("Sending signal " + std::to_string(signal) + " to process " + *it + "\n");

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
		(void)this->send("The process could not be found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	switch (p->getState())
	{
	case TM_P_RUNNING:
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_STARTING:
		ps.requested_state = TM_P_RUNNING;
		return (TM_POLL_CLIENT_OK);
	case TM_P_STOPPING:
		(void)this->send("The process is in transition state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_BACKOFF:
	case TM_P_FATAL:
		(void)this->send("The process is in a fatal state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_UNKNOWN:
		(void)this->send("Process is in an unknown state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_RUNNING;

	if (p->start())
	{
		(void)this->send("Failed to start the process\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_status(void)
{
	if (this->input.size() == 1 || this->input[1] == "all")
	{
		(void)this->send(this->_master.getProcsStatus());
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	std::vector<std::string> processes(this->input.begin() + 1, this->input.end());

	for (auto it = this->input.begin() + 1; it != this->input.end(); ++it)
	{
		auto p = this->_master.find(*it);
		if (!p)
		{
			(void)this->send("The process " + *it + " could not be found\n");
			continue;
		}

		(void)this->send(p->getStatus());
	}
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_stop(struct tm_pollclient_process_handler& ps)
{
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		(void)this->send("The process could not be found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	switch (p->getState())
	{
	case TM_P_STOPPING:
		ps.requested_state = TM_P_EXITED;
		return (TM_POLL_CLIENT_OK);
	case TM_P_STOPPED:
	case TM_P_EXITED:
		(void)this->send("The process is not running\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_FATAL:
	case TM_P_UNKNOWN:
		(void)this->send("Process is in an unknown state\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_EXITED;

	if (p->stop())
	{
		(void)this->send("Failed to stop the process\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_tail(void)
{
	if (this->input.size() < 2)
	{
		this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	auto p = this->_master.find(this->input[1]);
	if (!p)
	{
		this->send("The process could not be found\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}
	int fd = p->getStdOutFd();
	if (fd < 0)
	{
		this->send("Invalid file descriptor\n");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	off_t fileSize = lseek(fd, 0, SEEK_END);
	if (fileSize == -1)
	{
		Logger::perror("lseek error");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	off_t startPos = std::max(static_cast<off_t>(0), fileSize - 1600);
	if (lseek(fd, startPos, SEEK_SET) == -1)
	{
		Logger::perror("lseek error");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	char buf[TM_RECV_SIZE];
	ssize_t len;
	while ((len = read(fd, buf, TM_RECV_SIZE)) > 0)
	{
		(void)this->send(buf, len);
	}
	if (len == -1)
	{
		Logger::perror("read error");
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	// // 2. Suivre les nouvelles données
	// int flags = fcntl(fd, F_GETFL, 0);
	// if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	// {
	// 	Logger::perror("fcntl error");
	// 	return (TM_POLL_CLIENT_DISCONNECT);
	// }

	// struct pollfd pfd = { fd, POLLIN, 0 };

	// while (true)
	// {
	// 	int ret = ::poll(&pfd, 1, -1);
	// 	if (ret == -1)
	// 	{
	// 		Logger::perror("poll error");
	// 		break;
	// 	}

	// 	if (pfd.revents & POLLIN)
	// 	{
	// 		ssize_t len = read(fd, buf, TM_RECV_SIZE);
	// 		if (len >= 0)
	// 		{
	// 			buf[len] = '\0';
	// 			this->send(buf);
	// 		}
	// 		else if (errno != EAGAIN && errno != EWOULDBLOCK)
	// 		{
	// 			Logger::perror("read error");
	// 			break;
	// 		}
	// 	}
	// }

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_update(void)
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_version(void)
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage\n");
		return (TM_POLL_CLIENT_ERROR);
	}

	(void)this->send(TM_PROJECTD " " TM_VERSION " - " TM_AUTHOR "\n");
	return (TM_POLL_CLIENT_DISCONNECT);
}