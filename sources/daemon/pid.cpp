/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pid.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 13:34:11 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 13:34:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "logger/Logger.hpp"

static int g_pid_fd = -1;

int
create_pid_file(void)
{
	int32_t	tm_pid;
	int		pid_fd;

	tm_pid = getpid();

	pid_fd = open(TM_PID_FILE, O_WRONLY | O_CREAT, TM_DEFAULT_FILE_MODE);
	if (pid_fd == -1)
	{
		Logger::perror("open");
		return (TM_FAILURE);
	}

	if (flock(pid_fd, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            Logger::error("Another instance is already running!");
        } else {
            Logger::perror("flock");
        }
        close(pid_fd);
        return (TM_FAILURE);
    }

	ftruncate(pid_fd, 0);
	dprintf(pid_fd, "%d\n", tm_pid);

	g_pid_fd = pid_fd;

	return (TM_SUCCESS);
}

void
remove_pid_file(void)
{
	if (g_pid_fd != -1) {
		flock(g_pid_fd, LOCK_UN);
        close(g_pid_fd);
        g_pid_fd = -1;
    }

	if (unlink(TM_PID_FILE) == -1)
	{
		Logger::perror("unlink");
	}
}