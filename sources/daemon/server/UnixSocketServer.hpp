/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:45:59 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 19:02:51 by mgama            ###   ########.fr       */
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

struct tm_pollclient_process_handler {
	uint32_t	puid;
	int 		initial_state;
	int 		requested_state;
	bool		done;
	bool		group_request;
	const char*	success_message;
};

class UnixSocketServer: public UnixSocket
{
public:
	class Client
	{
	private:
		int			fd;
		bool		input_received;

		Taskmaster&	_master;
		std::vector<struct tm_pollclient_process_handler>	handlers;

		std::vector<std::string>	input;
		std::string					name;
		std::vector<std::string>	args;
		std::vector<std::string>	opts;

		int		_find_processes(const std::vector<std::string>& progs);

		int		_add(void);
		int		_avail(void);
		int		_clear(struct tm_pollclient_process_handler& p);
		int		_maintail(void);
		int		_pid(void);
		int		_reload(void);
		int		_remove(void);
		int		_reread(void);
		int		_restart(struct tm_pollclient_process_handler& p);
		int		_shutdown(void);
		int		_signal(struct tm_pollclient_process_handler& p);
		int		_start(struct tm_pollclient_process_handler& p);
		int		_status(void);
		int		_stop(struct tm_pollclient_process_handler& p);
		int		_tail(struct tm_pollclient_process_handler& p);
		int		_update(void);
		int		_version(void);

		int		_work(struct tm_pollclient_process_handler& ps);

		using ProcHandler = int (UnixSocketServer::Client::*)(struct tm_pollclient_process_handler&);
		using GenHandler = int (UnixSocketServer::Client::*)();
		static const std::unordered_map<std::string, ProcHandler> process_command_map;
		static const std::unordered_map<std::string, GenHandler> general_command_map;

	public:
		explicit Client(int fd, Taskmaster& master): fd(fd), input_received(false), _master(master) {}

		int		recv(void);
		int		send(const std::string& msg);
		int		send(const char* msg);
		int		send(const char* msg, size_t len);

		int		getFd(void) const;

		int		done(void);
		int		parse(const char* buff);
		int		exec(void);
	};

private:
	bool			_running;
	Taskmaster&		_master;

	std::map<int, tm_pollclient>	_poll_clients;

	int	serve(UnixSocketServer::Client& client);

	bool	_test_socket();

public:
	explicit UnixSocketServer(const char* socket_path, Taskmaster &master);
	~UnixSocketServer(void);

	int	listen(void);
	int	cycle(void);
	int	shutdown(void);
	int	stop(void);
};

#endif /* UNIXSOCKETSERVER_HPP */