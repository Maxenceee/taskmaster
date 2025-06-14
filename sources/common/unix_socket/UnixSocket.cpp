/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 15:26:16 by mgama             #+#    #+#             */
/*   Updated: 2025/06/14 10:51:41 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "unix_socket/UnixSocket.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"

UnixSocket::UnixSocket(const char* path): unix_domain_path(path), socket_path(resolve_path(path, "unix://")), sockfd(-1) {}

int
UnixSocket::poll(void)
{
	if (::poll(this->poll_fds.data(), this->poll_fds.size(), TM_POLL_TIMEOUT) == -1)
	{
		return (TM_FAILURE);
	}
	return (TM_SUCCESS);	
}

const std::string&
UnixSocket::getSocketPath(void) const
{
	return (this->socket_path);
}
