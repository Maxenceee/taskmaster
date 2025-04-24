/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:37:52 by mgama             #+#    #+#             */
/*   Updated: 2025/04/24 19:39:10 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIXSOCKET_HPP
#define UNIXSOCKET_HPP

#include "tm.hpp"

#define TM_RECV_SIZE	(2 << 12)

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

#define TM_POLL_EVENTS		POLLIN | POLLHUP | POLLERR
#define TM_POLL_NO_EVENTS	0

#define TM_DEFAULT_MAX_WORKERS	1024

#define TM_CR "\r"
#define TM_LF "\n"
#define TM_CRLF TM_CR TM_LF

class UnixSocket
{
protected:
	const char*			unix_domain_path;
	const std::string	socket_path;
	int 				sockfd;
	struct sockaddr_un	addr;

	std::vector<pollfd>	poll_fds;

	int	poll(void);

public:
	explicit UnixSocket(const char* path);
	virtual ~UnixSocket(void) = default;

	const std::string&	getSocketPath(void) const;
};

#endif /* UNIXSOCKET_HPP */