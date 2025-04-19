/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:45:59 by mgama             #+#    #+#             */
/*   Updated: 2025/04/19 11:13:58 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIXSOCKETSERVER_HPP
#define UNIXSOCKETSERVER_HPP

#include "unix_socket/UnixSocket.hpp"
#include "taskmaster/Taskmaster.hpp"

enum tm_polltype {
	TM_POLL_SERVER	= 0x00,	// A server socket
	TM_POLL_CLIENT	= 0x01	// A client socket
};

enum tm_pollclientstatus {
	TM_POLL_CLIENT_OK			= 0x00,
	TM_POLL_CLIENT_DISCONNECT	= 0x01,
	TM_POLL_CLIENT_CLOSED		= 0x02,
	TM_POLL_CLIENT_ERROR		= 0x03
};

typedef struct tm_pollclient {
	enum tm_polltype	type;
	void				*data;
} tm_pollclient;

#define TM_POLL_EVENTS		POLLIN | POLLHUP | POLLERR
#define TM_POLL_NO_EVENTS	0

class UnixSocketServer: public UnixSocket
{
private:
	bool		_running;
	Taskmaster	&_master;

	std::map<int, tm_pollclient>	_poll_clients;

	int	serve(int client);

public:
	UnixSocketServer(const char* socket_path, Taskmaster &master);
	~UnixSocketServer(void);

	int	listen(void);
	int	cycle(void);
	int	stop(void);
};

#endif /* UNIXSOCKETSERVER_HPP */