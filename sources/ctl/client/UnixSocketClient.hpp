/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClient.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:58:50 by mgama             #+#    #+#             */
/*   Updated: 2025/08/19 11:40:37 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIXSOCKETCLIENT_HPP
#define UNIXSOCKETCLIENT_HPP

#include "unix_socket/UnixSocket.hpp"

class UnixSocketClient: public UnixSocket
{
public:
	explicit UnixSocketClient(const char* socket_path);
	~UnixSocketClient(void);

	int	connect(void);

	ssize_t send(const char* msg, size_t len);
	ssize_t send(const std::string& msg);

	ssize_t 	print(bool forcenl = false);
	std::string recv(void);
};

#endif /* UNIXSOCKETCLIENT_HPP */