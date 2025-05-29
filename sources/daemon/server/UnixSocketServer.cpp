/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:43:04 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 21:57:20 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

UnixSocketServer::UnixSocketServer(const char* unix_path, Taskmaster& master): UnixSocket(unix_path), _running(false), _master(master)
{
	if (!this->_test_socket())
	{
		throw std::runtime_error("Another instance is already running!");
	}

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

	auto conf = this->_master.getServerConf();

	if (chmod(this->socket_path.c_str(), conf.chmod) == -1)
	{
		Logger::perror("unix server: chmod failed");
		throw std::runtime_error("could not chmod socket");
	}

	if (chown(this->socket_path.c_str(), conf.chown.uid, conf.chown.gid) == -1)
	{
		Logger::perror("unix server: chown failed");
		throw std::runtime_error("could not socket owner");
	}
}

UnixSocketServer::~UnixSocketServer(void)
{
	if (this->_running)
	{
		(void)this->shutdown();
		(void)this->stop();
	}
	std::cout << " ~UnixSocketServer called" << std::endl;
}

bool
UnixSocketServer::_test_socket()
{
	if (access(this->socket_path.c_str(), F_OK) != 0)
	{
		return (true);
	}

	struct stat st;
	if (stat(this->socket_path.c_str(), &st) != 0)
	{
		return (false);
	}

	if (!S_ISSOCK(st.st_mode))
	{
		return (false);
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

		if (write(test_fd, "ping", 4) == -1)
		{
			usable = false;
		}
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
			auto c = reinterpret_cast<UnixSocketServer::Client *>(client.second.data);
			if (false == Taskmaster::reload)
			{
				(void)c->send("Good bye!");
				(void)c->send(TM_CRLF);
			}
			(void)::shutdown(c->getFd(), SHUT_RDWR);
			delete c;
		}
	}
	for (auto& poll_fd : this->poll_fds)
	{
		(void)close(poll_fd.fd);
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
		if (errno == EINTR)
		{
			return (TM_SUCCESS);
		}
		Logger::perror("server error: an error occurred while poll'ing");
		return (TM_FAILURE);
	}

	UnixSocketServer::Client* client;

	for (size_t i = 0; i < this->poll_fds.size(); ++i)
	{
		if (this->poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
		{
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
				this->_poll_clients[newclient] = (tm_pollclient){TM_POLL_CLIENT, new UnixSocketServer::Client(newclient, this->_master)};
				this->poll_fds.push_back((pollfd){newclient, TM_POLL_EVENTS, TM_POLL_NO_EVENTS});
				break;

			case TM_POLL_CLIENT:
				client = reinterpret_cast<UnixSocketServer::Client *>(this->_poll_clients[this->poll_fds[i].fd].data);
				if (!client || this->serve(*client) != TM_POLL_CLIENT_OK)
				{
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
	return (client.recv());
}
