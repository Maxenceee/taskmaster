/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClient.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:59:19 by mgama             #+#    #+#             */
/*   Updated: 2025/01/31 16:00:16 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "unix_socket/client/UnixSocketClient.hpp"

UnixSocketClient::UnixSocketClient(const char* socket_path): UnixSocket(socket_path)
{
	if ((this->sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket creation failed");
		throw std::runtime_error("socket creation failed");
	}

	bzero(&this->addr, sizeof(this->addr));
	this->addr.sun_family = AF_UNIX;
	strncpy(this->addr.sun_path, socket_path, sizeof(this->addr.sun_path) - 1);
}

UnixSocketClient::~UnixSocketClient(void)
{
	close(this->sockfd);
}