/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocket.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/01 15:26:16 by mgama             #+#    #+#             */
/*   Updated: 2025/02/01 15:28:24 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "unix_socket/UnixSocket.hpp"
#include "logger/Logger.hpp"

UnixSocket::UnixSocket(const char* path): socket_path(path), sockfd(-1) {}

int
UnixSocket::poll(void)
{
	if (::poll(this->poll_fds.data(), this->poll_fds.size(), TM_POLL_TIMEOUT) == -1)
	{
		if (errno == EINTR) {
			return (TM_SUCCESS);
		}
		Logger::perror("server error: an error occurred while poll'ing");
		return (TM_FAILURE);
	}
	return (TM_SUCCESS);	
}
