/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketClient.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 15:58:50 by mgama             #+#    #+#             */
/*   Updated: 2025/01/31 16:11:15 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIXSOCKETCLIENT_HPP
#define UNIXSOCKETCLIENT_HPP

#include "unix_socket/UnixSocket.hpp"

class UnixSocketClient: public UnixSocket
{
public:
	UnixSocketClient(const char* socket_path);
	~UnixSocketClient(void);

	int send(const char* msg);
	int send(const std::string& msg);
};

#endif /* UNIXSOCKETCLIENT_HPP */