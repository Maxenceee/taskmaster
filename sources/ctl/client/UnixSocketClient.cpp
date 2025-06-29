/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClient.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:59:19 by mgama             #+#    #+#             */
/*   Updated: 2025/06/14 10:54:01 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client/UnixSocketClient.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

UnixSocketClient::UnixSocketClient(const char* socket_path): UnixSocket(socket_path)
{
	this->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (this->sockfd < 0)
	{
		Logger::perror("socket creation failed");
		throw std::runtime_error("socket creation failed");
	}

	bzero(&this->addr, sizeof(this->addr));
	this->addr.sun_family = AF_UNIX;
	(void)strncpy(this->addr.sun_path, this->socket_path.c_str(), sizeof(this->addr.sun_path) - 1);
}

UnixSocketClient::~UnixSocketClient(void)
{
	(void)close(this->sockfd);
}

int
UnixSocketClient::connect(void)
{
	if (::connect(this->sockfd, (struct sockaddr*)&this->addr, sizeof(this->addr)) == -1)
	{
		return (TM_FAILURE);
	}

	this->poll_fds.push_back((pollfd){this->sockfd, TM_POLL_EVENTS, TM_POLL_NO_EVENTS});

	return (TM_SUCCESS);
}

ssize_t
UnixSocketClient::send(const char* msg, size_t len)
{
	return (::send(this->sockfd, msg, len, 0));
}

ssize_t
UnixSocketClient::send(const std::string& msg)
{
	return (this->send(msg.c_str(), msg.length()));
}

ssize_t
UnixSocketClient::print(void)
{
	char buffer[1024];
	ssize_t total_bytes = 0;

	while (true)
	{
		if (this->poll() == TM_FAILURE)
		{
			if (errno == EINTR)
			{
				std::cout << "\b\b"; // Remove ^C
				std::cout << "Signal received, process has been moved to background task.\n";
				break;
			}
			return (TM_FAILURE);
		}

		if (this->poll_fds[0].revents & POLLIN)
		{
			ssize_t n = ::recv(this->sockfd, buffer, sizeof(buffer) - 1, 0);
			if (n <= 0)
			{
				if (n < 0)
				{
					Logger::perror("recv failed");
				}
				break;
			}

			buffer[n] = '\0';
			std::cout << buffer << std::flush;
			total_bytes += n;
		}
		else if (this->poll_fds[0].revents & (POLLHUP | POLLERR | POLLNVAL))
		{
			break;
		}
	}

	return (total_bytes);
}

std::string
UnixSocketClient::recv(void)
{
	char buffer[1024];

	std::string response;

	while (true)
	{
		if (this->poll() == TM_FAILURE)
		{
			return ("");
		}

		if (this->poll_fds[0].revents & POLLIN)
		{
			ssize_t n = ::recv(this->sockfd, buffer, sizeof(buffer) - 1, 0);
			if (n <= 0)
			{
				break;
			}

			response.append(buffer, n);
		}
		else if (this->poll_fds[0].revents & (POLLHUP | POLLERR | POLLNVAL))
		{
			break;
		}
	}

	return (response);
}
