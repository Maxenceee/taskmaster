/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClient.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:59:19 by mgama             #+#    #+#             */
/*   Updated: 2025/02/04 11:04:24 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "unix_socket/client/UnixSocketClient.hpp"
#include "logger/Logger.hpp"

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
	(void)strncpy(this->addr.sun_path, socket_path, sizeof(this->addr.sun_path) - 1);
}

UnixSocketClient::~UnixSocketClient(void)
{
	(void)close(this->sockfd);
}