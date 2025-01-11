/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:13:48 by mgama             #+#    #+#             */
/*   Updated: 2025/01/11 13:17:12 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TASKMASTER_HPP
#define TASKMASTER_HPP

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <spawn.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netdb.h>
#include <poll.h>
#include <arpa/inet.h>

#include <iostream>

#define WBS_SOCKET_PATH "./taskmaster.sock"

// spawn
int	spawn_child(char *const *argv, char *const *envp, int redirect_fd);

#endif /* TASKMASTER_HPP */