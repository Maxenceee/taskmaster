/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:43:04 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 13:40:42 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

UnixSocketServer::UnixSocketServer(const char* unix_path, const Taskmaster& master): UnixSocket(unix_path), _master(master)
{
	if (!this->_test_socket())
		throw std::runtime_error("Another instance is already running!");

	this->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (this->sockfd == -1)
	{
		Logger::perror("unix server: socket creation failed");
		throw std::runtime_error("socket creation failed");
	}

	int option = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1)
	{
		(void)close(this->sockfd);
		Logger::perror("unix server: setsockopt failed");
		throw std::runtime_error("setsockopt failed");
	}

	(void)unlink(this->socket_path.c_str());
	bzero(&this->addr, sizeof(this->addr));
	this->addr.sun_family = AF_UNIX;
	(void)strncpy(this->addr.sun_path, this->socket_path.c_str(), sizeof(this->addr.sun_path) - 1);

	if (bind(this->sockfd, (struct sockaddr *) &this->addr, sizeof(struct sockaddr_un)) == -1)
	{
		if (errno == EADDRINUSE)
		{
			Logger::error("unix server: bind error: Address already in use");
		}
		else
		{
			Logger::perror("unix server: bind error");
		}
		throw std::runtime_error("could not bind socket to address");
	}
}

UnixSocketServer::~UnixSocketServer(void)
{
	if (this->_running)
	{
		(void)this->shutdown();
		(void)this->stop();
	}
}

bool
UnixSocketServer::_test_socket()
{
	if (access(this->socket_path.c_str(), F_OK) != 0)
	{
		return (true);
	}

	int test_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (test_fd == -1)
	{
		Logger::perror("unix server: socket creation for testing failed");
		return (false);
	}

	struct sockaddr_un test_addr;
	bzero(&this->addr, sizeof(this->addr));
	test_addr.sun_family = AF_UNIX;
	(void)strncpy(test_addr.sun_path, this->socket_path.c_str(), sizeof(test_addr.sun_path) - 1);

	bool usable = true;
	if (connect(test_fd, (struct sockaddr*)&test_addr, sizeof(test_addr)) == 0)
	{
		usable = false;
	}
	else
	{
		if (errno == ECONNREFUSED || errno == ENOENT)
		{
			usable = true;
		}
		else
		{
			usable = false;
		}
	}

	(void)close(test_fd);
	return (usable);
}

int
UnixSocketServer::listen(void)
{
	if (::listen(this->sockfd, TM_DEFAULT_MAX_WORKERS) == -1)
	{
		Logger::perror("unix server: listen failed");
		return (TM_FAILURE);
	}
	this->poll_fds.push_back((pollfd){this->sockfd, TM_POLL_EVENTS, TM_POLL_NO_EVENTS});
	this->_poll_clients[this->sockfd] = (tm_pollclient){TM_POLL_SERVER, nullptr};

	this->_running = true;
	return (TM_SUCCESS);
}

int
UnixSocketServer::shutdown(void)
{
	Logger::debug("Removing socket file: " + this->socket_path);
	(void)unlink(this->socket_path.c_str());

	this->_running = false;

	return (TM_SUCCESS);
}

int
UnixSocketServer::stop(void)
{
	for (auto& client : this->_poll_clients)
	{
		if (client.second.type == TM_POLL_CLIENT)
		{
			delete reinterpret_cast<UnixSocketServer::Client *>(client.second.data);
		}
	}
	for (auto& poll_fd : this->poll_fds)
	{
		(void)close(poll_fd.fd);
	}

	return (TM_SUCCESS);
}

int
read_from_client(int client_fd, char *buffer, size_t buffer_size)
{
	ssize_t	bytes_received;

	bytes_received = recv(client_fd, buffer, buffer_size, 0);
	if (bytes_received == -1)
	{
		Logger::perror("server error: an error occurred while receiving data from the client");
		return (TM_FAILURE);
	}
	else if (bytes_received == 0)
	{
		Logger::debug("Connection closed by the client");
		return (TM_FAILURE);
	}
	return (TM_SUCCESS);
}

int
UnixSocketServer::cycle()
{
	int newclient;
	std::vector<int>	to_remove;

	if (this->poll() == TM_FAILURE)
	{
		if (errno != EINTR)
		{
			Logger::perror("server error: an error occurred while poll'ing");
		}
		return (TM_FAILURE);
	}

	UnixSocketServer::Client* client;

	for (size_t i = 0; i < this->poll_fds.size(); ++i)
	{
		if (this->poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
		{
			Logger::debug("Socket error (POLLERR / POLLHUP / POLLNVAL detected)");
			to_remove.push_back(i);
		}
		else if (this->poll_fds[i].revents & POLLIN)
		{
			switch (this->_poll_clients[this->poll_fds[i].fd].type)
			{
			case TM_POLL_SERVER:
				newclient = accept(this->sockfd, nullptr, nullptr);
				if (newclient == -1)
				{
					Logger::perror("server error: accept failed");
					return (TM_SUCCESS);
				}
				Logger::debug("New client connected");
				this->_poll_clients[newclient] = (tm_pollclient){TM_POLL_CLIENT, new UnixSocketServer::Client(newclient, this->_master)};
				this->poll_fds.push_back((pollfd){newclient, TM_POLL_EVENTS, TM_POLL_NO_EVENTS});
				break;

			case TM_POLL_CLIENT:
				client = reinterpret_cast<UnixSocketServer::Client *>(this->_poll_clients[this->poll_fds[i].fd].data);
				if (!client || this->serve(*client) != TM_POLL_CLIENT_OK)
				{
					Logger::debug("Client disconnected");
					to_remove.push_back(i);
				}
				break;
			}
		}
		else if (this->_poll_clients[this->poll_fds[i].fd].type == TM_POLL_CLIENT)
		{
			client = reinterpret_cast<UnixSocketServer::Client *>(this->_poll_clients[this->poll_fds[i].fd].data);
			if (!client || client->done() != TM_POLL_CLIENT_OK)
			{
				to_remove.push_back(i);
			}
		}
	}

	int client_fd;
	for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it)
	{
		client_fd = this->poll_fds[*it].fd;
		(void)::shutdown(client_fd, SHUT_RDWR);
		(void)close(client_fd);
		client = reinterpret_cast<UnixSocketServer::Client *>(this->_poll_clients[client_fd].data);
		delete client;
		this->poll_fds.erase(this->poll_fds.begin() + *it);
		this->_poll_clients.erase(client_fd);
	}
	return (TM_SUCCESS);
}

int
UnixSocketServer::serve(UnixSocketServer::Client& client)
{
	char	buffer[TM_RECV_SIZE];

	if (read_from_client(client.getFd(), buffer, TM_RECV_SIZE))
	{
		return (TM_POLL_CLIENT_ERROR);
	}

	if (client.parse(buffer))
	{
		return (TM_POLL_CLIENT_ERROR);
	}

	return (TM_POLL_CLIENT_OK);
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

	auto p = this->_master.get(this->puid);
	if (!p)
	{
		Logger::error("The process could not be found");
		return (TM_POLL_CLIENT_ERROR);
	}

	if (p->reachedDesiredState())
	{
		std::stringstream ss;
		ss << "Process " << p->getPid() << " (" << p->getProgramName() << ") is now " << Process::getStateName(p->getState()) << " [" << Process::getStateName(p->getDesiredState()) << "]" << "\n";

		this->send(ss.str());
		return (TM_POLL_CLIENT_DISCONNECT);	
	}

	if (p->getDesiredState() != this->requested_state)
	{
		this->requested_state = p->getDesiredState();
		this->send("Request interrupted by another request\n");
		return (TM_POLL_CLIENT_OK);
	}

	if (p->getState() == TM_P_FATAL || p->getState() == TM_P_UNKNOWN)
	{
		Logger::error("The process is in a fatal state");
		this->send("The process is in a fatal state\n");
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
		if (this->input[0] == "shutdown")
		{
			this->send("Shutting down the daemon\n");
			Taskmaster::running = false;
			return (TM_POLL_CLIENT_OK);
		}
		else if (this->input[0] == "reload")
		{
			this->send("Reloading the daemon\n");
			this->_master.restart();
			return (TM_POLL_CLIENT_DISCONNECT);
		}
		else if (this->input[0] == "status")
		{
			this->send(this->_master.getStatus());
			return (TM_POLL_CLIENT_DISCONNECT);
		}
		else if (this->input[0] == "version")
		{
			this->send(TM_PROJECTD " version " TM_VERSION " - " TM_AUTHOR "\n");
			return (TM_POLL_CLIENT_DISCONNECT);
		}

		auto p = this->_master.find(this->input[1]);
		if (!p)
		{
			Logger::error("The process could not be found");
			this->send("The process could not be found\n");
			return (TM_POLL_CLIENT_ERROR);
		}
		this->puid = p->getUid();
		this->initial_state = p->getState();

		if (this->input[0] == "start")
		{
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
			this->requested_state = TM_P_RUNNING;
			(void)p->start();
		}
		else if (this->input[0] == "restart")
		{
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
			this->requested_state = TM_P_RUNNING;
			(void)p->restart();
		}
		else if (this->input[0] == "stop")
		{
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
			this->requested_state = TM_P_EXITED;
			(void)p->stop();
		}
		else if (this->input[0] == "signal")
		{
			int signal = std::stoi(this->input[2]);
			if (p->getState() != TM_P_RUNNING)
			{
				this->send("The process is not running\n");
				return (TM_POLL_CLIENT_DISCONNECT);
			}
			this->send("Sending signal " + std::to_string(signal) + " to process " + this->input[1] + "\n");
			(void)p->signal(signal);
			return (TM_POLL_CLIENT_DISCONNECT);
		}
	}

	return (TM_POLL_CLIENT_OK);
}