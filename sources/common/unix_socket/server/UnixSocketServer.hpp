/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:45:59 by mgama             #+#    #+#             */
/*   Updated: 2025/01/31 16:32:53 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIXSOCKETSERVER_HPP
#define UNIXSOCKETSERVER_HPP

#include "unix_socket/UnixSocket.hpp"

class UnixSocketServer: public UnixSocket
{
public:
	UnixSocketServer(const char* socket_path);
	~UnixSocketServer(void);

	int	listen(void);
	int	poll(void);
};

#endif /* UNIXSOCKETSERVER_HPP */