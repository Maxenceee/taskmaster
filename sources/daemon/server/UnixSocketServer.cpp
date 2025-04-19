/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:43:04 by mgama             #+#    #+#             */
/*   Updated: 2025/04/19 11:21:04 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

UnixSocketServer::UnixSocketServer(const char* unix_path, Taskmaster &master): UnixSocket(unix_path), _master(master)
{
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
		(void)this->stop();
	}
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
UnixSocketServer::stop(void)
{
	(void)close(this->sockfd);

	for (auto& poll_fd : this->poll_fds)
	{
		(void)close(poll_fd.fd);
	}

	Logger::debug("Removing socket file: " + this->socket_path);
	(void)unlink(this->socket_path.c_str());

	this->_running = false;

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
	Logger::debug("Data received: " + std::string(buffer, bytes_received));
	return (TM_SUCCESS);
}

int
UnixSocketServer::cycle()
{
	int newclient;
	std::vector<int>	to_remove;

	if (this->poll() == TM_FAILURE)
	{
		return (TM_FAILURE);
	}

	for (size_t i = 0; i < this->poll_fds.size(); ++i)
	{
		if (this->poll_fds[i].revents & POLLHUP)
		{
			Logger::debug("Connection closed by the client (event POLLHUP)");
			to_remove.push_back(i);
		}
		else if (this->poll_fds[i].revents & POLLERR)
		{
			Logger::debug("Socket error (POLLERR detected)");
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
				this->_poll_clients[newclient] = (tm_pollclient){TM_POLL_CLIENT, nullptr};
				this->poll_fds.push_back((pollfd){newclient, TM_POLL_EVENTS, TM_POLL_NO_EVENTS});
				break;
			
			case TM_POLL_CLIENT:
				(void)serve(this->poll_fds[i].fd);
				to_remove.push_back(i);
				break;
			}
		}
	}

	for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it)
	{
		(void)close(this->poll_fds[*it].fd);
		this->poll_fds.erase(this->poll_fds.begin() + *it);
		this->_poll_clients.erase(this->poll_fds[*it].fd);
	}
	return (TM_SUCCESS);
}

int
UnixSocketServer::serve(int client_fd)
{
	char	buffer[TM_RECV_SIZE];

	(void)read_from_client(client_fd, buffer, TM_RECV_SIZE);
	if (strncmp(buffer, "kill", 4) == 0)
	{
		Logger::debug("Client requested to exit");
		Taskmaster::running = false;
	}
	if (strncmp(buffer, "status", 4) == 0)
	{
		Logger::debug("Client requested status");
		const std::string& s = this->_master.getStatus();
		(void)send(client_fd, s.c_str(), s.size(), 0);
		return (TM_SUCCESS);
	}

	(void)send(client_fd, "Data successfully received", 27, 0);
	return (TM_SUCCESS);
}