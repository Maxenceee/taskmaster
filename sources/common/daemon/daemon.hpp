/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 15:29:56 by mgama             #+#    #+#             */
/*   Updated: 2025/02/04 22:43:32 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DAEMON_HPP
#define DAEMON_HPP

#include "tm.hpp"

#define TM_DEFAULT			00		// Don't fork
#define TM_NO_CHDIR			01		// Don't chdir ("/")
#define TM_CLOSE_FILES		02		// Don't close all open files
#define TM_NO_UMASK0		04		// Don't do a umask(0)
#define TM_MAX_CLOSE		8192	// Max file descriptors to close if sysconf(_SC_OPEN_MAX) is indeterminate

// returns 0 on success -1 on error
int become_daemon(int flags);

#endif /* DAEMON_HPP */