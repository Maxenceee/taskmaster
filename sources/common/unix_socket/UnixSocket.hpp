/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:37:52 by mgama             #+#    #+#             */
/*   Updated: 2025/01/31 16:41:14 by mgama            ###   ########.fr       */
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

enum tm_polltype {
	WBS_POLL_SERVER	= 0x00,	// A server socket
	WBS_POLL_CLIENT	= 0x01	// A client socket
};

enum tm_pollclientstatus {
	WBS_POLL_CLIENT_OK			= 0x00,
	WBS_POLL_CLIENT_DISCONNECT	= 0x01,
	WBS_POLL_CLIENT_CLOSED		= 0x02,
	WBS_POLL_CLIENT_ERROR		= 0x03
};

struct tm_pollclient {
	enum tm_polltype	type;
	void				*data;
};

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