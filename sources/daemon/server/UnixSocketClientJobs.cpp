/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClientJobs.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:46:05 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 21:05:12 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

const std::unordered_map<std::string, UnixSocketServer::Client::ProcHandler> UnixSocketServer::Client::process_command_map = {
	{"clear", &UnixSocketServer::Client::_clear},
	{"restart", &UnixSocketServer::Client::_restart},
	{"signal", &UnixSocketServer::Client::_signal},
	{"start", &UnixSocketServer::Client::_start},
	{"stop", &UnixSocketServer::Client::_stop},
	{"tail", &UnixSocketServer::Client::_tail}
};

const std::unordered_map<std::string, UnixSocketServer::Client::GenHandler> UnixSocketServer::Client::general_command_map = {
	{"add", &UnixSocketServer::Client::_add},
	{"avail", &UnixSocketServer::Client::_avail},
	{"maintail", &UnixSocketServer::Client::_maintail},
	{"pid", &UnixSocketServer::Client::_pid},
	{"reload", &UnixSocketServer::Client::_reload},
	{"remove", &UnixSocketServer::Client::_remove},
	{"reread", &UnixSocketServer::Client::_reread},
	{"shutdown", &UnixSocketServer::Client::_shutdown},
	{"status", &UnixSocketServer::Client::_status},
	{"update", &UnixSocketServer::Client::_update},
	{"version", &UnixSocketServer::Client::_version}
};

const std::unordered_map<std::string, const char*> tm_pollclient_delayed_success_message = {
	{"clear", "cleared"},
	{"restart", "restarted"},
	{"signal", "signalled"},
	{"start", "started"},
	{"stop", "stopped"},
	{"nrun", "is not running"}
};

int
UnixSocketServer::Client::_find_processes(const std::vector<std::string>& progs)
{
	if (progs.empty())
	{
		(void)this->send("Invalid usage: No process specified");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	if (progs.size() == 1 && progs[0] == "all")
	{
		for (const auto& p : this->_master.all())
		{
			this->handlers.push_back({p->getPuid(), p->getState(), -1, false, true, nullptr});
		}
		if (this->handlers.empty())
		{
			(void)this->send("No process found");
			(void)this->send(TM_CRLF);
			return (TM_POLL_CLIENT_DISCONNECT);
		}
		return (TM_POLL_CLIENT_OK);
	}

	std::unordered_set<std::string> seen;
	for (const auto& prog : progs)
	{
		if (seen.find(prog) != seen.end())
			continue;
		seen.insert(prog);

		auto p = this->_master.find(prog);
		if (!p)
		{
			(void)this->send("The process ");
			(void)this->send(prog);
			(void)this->send(" could not be found");
			(void)this->send(TM_CRLF);
			continue;
		}

		this->handlers.push_back({p->getPuid(), p->getState(), -1, false, false, nullptr});
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_add(void)
{
	for (auto a : this->args)
	{
		if (this->_master.add(a))
		{
			(void)this->send("Failed to add the process: ");
			(void)this->send(a);
			(void)this->send(TM_CRLF);
		}
		else
		{
			(void)this->send(a);
			(void)this->send(": added process group");
			(void)this->send(TM_CRLF);
		}
	}
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_avail(void)
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	(void)this->send(this->_master.getAvailableProcs());

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_clear(struct tm_pollclient_process_handler& ps)
{
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		(void)this->send("The process could not be found");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.success_message = tm_pollclient_delayed_success_message.at("clear");

	if (p->clearLogFiles() == TM_FAILURE)
	{
		(void)this->send(p->getProcessName());
		(void)this->send(": failed to clear the log files");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = p->getState();

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
	if (this->args.size() > 1)
	{
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	if (this->args.size() == 0)
	{
		(void)this->send("Daemon pid: ");
		(void)this->send(std::to_string(getpid()));
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}
	
	if (this->args[0] == "all")
	{
		for (const auto& p : this->_master.all())
		{
			(void)this->send("Process ");
			(void)this->send(p->getProcessName());
			(void)this->send(" pid: ");
			(void)this->send(std::to_string(p->getPid()));
			(void)this->send(TM_CRLF);
		}
	}
	else
	{
		auto p = this->_master.find(this->args[0]);
		if (!p)
		{
			(void)this->send("The process could not be found");
			(void)this->send(TM_CRLF);
			return (TM_POLL_CLIENT_DISCONNECT);
		}
		(void)this->send("Process ");
		(void)this->send(p->getProcessName());
		(void)this->send(" pid: ");
		(void)this->send(std::to_string(p->getPid()));
		(void)this->send(TM_CRLF);
	}
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_reload()
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	Logger::print("Reloading the daemon...");
	(void)this->send("Reloading the daemon...");
	(void)this->send(TM_CRLF);
	Taskmaster::running = false;
	Taskmaster::reload = true;

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_remove(void)
{
	for (auto a : this->args)
	{
		if (this->_master.remove(a))
		{
			(void)this->send("Failed to remove the process: ");
			(void)this->send(a);
			(void)this->send(TM_CRLF);
		}
		else
		{
			(void)this->send(a);
			(void)this->send(": removed process group");
			(void)this->send(TM_CRLF);
		}
	}
	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_reread(void)
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	if (this->_master.readconfig() == TM_FAILURE)
	{
		(void)this->send("Failed to reread the config file");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	(void)this->send(this->_master.getConfChanges());

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_restart(struct tm_pollclient_process_handler& ps)
{	
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		(void)this->send("The process could not be found");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.success_message = tm_pollclient_delayed_success_message.at("restart");

	switch (p->getState())
	{
	case TM_P_STOPPING:
		if (false == ps.group_request)
		{
			(void)this->send("The process is in transition state");
			(void)this->send(TM_CRLF);
		}
		return (TM_POLL_CLIENT_DISCONNECT); 
	case TM_P_BACKOFF:
	// case TM_P_FATAL:
		if (false == ps.group_request)
		{
			(void)this->send("The process is in a fatal state");
			(void)this->send(TM_CRLF);
		}
		return (TM_POLL_CLIENT_DISCONNECT);
	// case TM_P_UNKNOWN:
	// 	if (false == ps.group_request)
	// 	{
	// 		(void)this->send("Process is in an unknown state");
	// 		(void)this->send(TM_CRLF);	
	// 	}
	// 	return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_RUNNING;

	if (p->restart())
	{
		(void)this->send("Failed to restart the process");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_shutdown(void)
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	(void)this->send("Shutting down the daemon...");
	(void)this->send(TM_CRLF);
	Taskmaster::running = false;

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_signal(struct tm_pollclient_process_handler& ps)
{
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		(void)this->send("The process could not be found");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.success_message = tm_pollclient_delayed_success_message.at("signal");

	int signal;
	try
	{
		signal = std::stoi(this->opts[0]);
	}
	catch(...)
	{
		(void)this->send("Invalid signal");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	if (signal < 1 || signal > NSIG)
	{
		(void)this->send("Invalid signal");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	if (p->getState() != TM_P_RUNNING)
	{
		(void)this->send("The process is not running");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	(void)p->signal(signal);

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_start(struct tm_pollclient_process_handler& ps)
{
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		(void)this->send("The process could not be found");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.success_message = tm_pollclient_delayed_success_message.at("start");

	switch (p->getState())
	{
	case TM_P_RUNNING:
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_STARTING:
		ps.requested_state = TM_P_RUNNING;
		return (TM_POLL_CLIENT_OK);
	case TM_P_STOPPING:
		if (false == ps.group_request)
		{
			(void)this->send("The process is in transition state");
			(void)this->send(TM_CRLF);
		}
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_BACKOFF:
	// case TM_P_FATAL:
		if (false == ps.group_request)
		{
			(void)this->send("The process is in a fatal state");
			(void)this->send(TM_CRLF);
		}
		return (TM_POLL_CLIENT_DISCONNECT);
	// case TM_P_UNKNOWN:
	// 	if (false == ps.group_request)
	// 	{
	// 		(void)this->send("Process is in an unknown state");
	// 		(void)this->send(TM_CRLF);
	// 	}
	// 	return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_RUNNING;

	if (p->start())
	{
		(void)this->send("Failed to start the process");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_status(void)
{
	if (this->args.size() == 0 || (this->args.size() == 1 && this->args[0] == "all"))
	{
		(void)this->send(this->_master.getProcsStatus());
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	for (auto it = this->args.begin(); it != this->args.end(); ++it)
	{
		auto p = this->_master.find(*it);
		if (!p)
		{
			(void)this->send("The process ");
			(void)this->send(*it);
			(void)this->send(" could not be found");
			(void)this->send(TM_CRLF);
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
		(void)this->send("The process could not be found");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.success_message = tm_pollclient_delayed_success_message.at("stop");

	switch (p->getState())
	{
	case TM_P_STOPPING:
		ps.requested_state = TM_P_EXITED;
		return (TM_POLL_CLIENT_OK);
	case TM_P_STOPPED:
	case TM_P_EXITED:
		if (false == ps.group_request)
		{
			(void)this->send("The process is not running");
			(void)this->send(TM_CRLF);
		}
		return (TM_POLL_CLIENT_DISCONNECT);
	case TM_P_FATAL:
	case TM_P_UNKNOWN:
		if (false == ps.group_request)
		{
			(void)this->send("Process is in an unknown state");
			(void)this->send(TM_CRLF);
		}
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	ps.requested_state = TM_P_EXITED;

	if (p->stop())
	{
		(void)this->send("Failed to stop the process");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	return (TM_POLL_CLIENT_OK);
}

int
UnixSocketServer::Client::_tail(struct tm_pollclient_process_handler& ps)
{
	auto p = this->_master.get(ps.puid);
	if (!p)
	{
		(void)this->send("The process could not be found");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}

	bool isStdOut = this->opts.empty() || this->opts[0] == "stdout";
	if (!isStdOut && this->opts[0] != "stderr")
	{
		(void)this->send("Invalid channel");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	int fd = isStdOut ? p->getStdOutFd() : p->getStdErrFd();
	if (fd < 0)
	{
		this->send("Invalid file descriptor");
		(void)this->send(TM_CRLF);
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
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	if (this->_master.readconfig() == TM_FAILURE)
	{
		(void)this->send("Failed to read config file");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_DISCONNECT);
	}
	
	(void)this->send(this->_master.update());

	return (TM_POLL_CLIENT_DISCONNECT);
}

int
UnixSocketServer::Client::_version(void)
{
	if (this->input.size() > 1)
	{
		(void)this->send("Invalid usage");
		(void)this->send(TM_CRLF);
		return (TM_POLL_CLIENT_ERROR);
	}

	(void)this->send(TM_PROJECTD " " TM_VERSION " - " TM_AUTHOR "");
	(void)this->send(TM_CRLF);
	return (TM_POLL_CLIENT_DISCONNECT);
}
