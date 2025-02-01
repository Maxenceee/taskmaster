/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:37:52 by mgama             #+#    #+#             */
/*   Updated: 2025/02/01 00:08:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIXSOCKET_HPP
#define UNIXSOCKET_HPP

#include "tm.hpp"

#define TM_RECV_SIZE	2 << 12

#ifdef REQUEST_TIMEOUT
#define TM_REQUEST_DEFAULT_TIMEOUT	REQUEST_TIMEOUT
#else
// default timeout duration, 1 minute in milliseconds
#define TM_REQUEST_DEFAULT_TIMEOUT	60000
#endif /* REQUEST_TIMEOUT */

#ifdef POLL_TIMEOUT
#define TM_POLL_TIMEOUT	POLL_TIMEOUT
#else
#define TM_POLL_TIMEOUT	100
#endif /* POLL_TIMEOUT */

// default timeout duration, 1 minute in seconds
#define WBD_PROXY_SELECT_TIMEOUT	60

#define TM_DEFAULT_MAX_WORKERS	1024

#define TM_CRLF "\r\n"

class UnixSocket
{
protected:
	const char*			socket_path;
	int 				sockfd;
	struct sockaddr_un	addr;

	std::vector<pollfd>				poll_fds;

public:
	explicit UnixSocket(const char* path): socket_path(path), sockfd(-1) {}
	virtual ~UnixSocket(void) = default;

	virtual int	poll(void) = 0;
};

#endif /* UNIXSOCKET_HPP */