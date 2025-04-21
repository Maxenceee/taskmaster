/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:45:59 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 12:31:50 by mgama            ###   ########.fr       */
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

class UnixSocketServer: public UnixSocket
{
public:
	class Client
	{
	private:
		int			fd;
		bool		input_received;
		
		const Taskmaster&	_master;
		int initial_state;
		int requested_state;
		std::string puid;

		std::vector<std::string>		input;

	public:
		Client(int fd, const Taskmaster& master): fd(fd), input_received(false), _master(master) {}

		int		send(const std::string& msg);
		int		send(const char* msg);

		int		getFd(void) const;
		
		int		done();
		int		parse(const char* buff);
	};

private:
	bool				_running;
	const Taskmaster&	_master;

	std::map<int, tm_pollclient>	_poll_clients;

	int	serve(UnixSocketServer::Client& client);

	bool	_test_socket();

public:
	UnixSocketServer(const char* socket_path, const Taskmaster &master);
	~UnixSocketServer(void);

	int	listen(void);
	int	cycle(void);
	int	stop(void);
};

#endif /* UNIXSOCKETSERVER_HPP */