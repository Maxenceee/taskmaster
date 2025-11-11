/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pid.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 13:34:11 by mgama             #+#    #+#             */
/*   Updated: 2025/11/11 14:21:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "logger/Logger.hpp"

static int g_pid_fd = -1;

static int
signal_pid(int fd)
{
	char buffer[TM_INT64_LEN];
	if (read(fd, buffer, TM_INT64_LEN) == -1)
	{
		Logger::perror("read");
		return (TM_FAILURE);
	}

	int old_pid = atoi(buffer);
	if (old_pid > 0)
	{
		if (kill(old_pid, 0) == 0)
		{
			return (TM_FAILURE);
		}
	}

	return (TM_SUCCESS);
}

static int lock_pid(int fd)
{
	if (flock(fd, LOCK_EX | LOCK_NB) == -1)
	{
		if (errno == EWOULDBLOCK)
		{
			if (signal_pid(fd) == TM_SUCCESS)
			{
				return (TM_SUCCESS);
			}
			Logger::error("Another instance is already running!");
		}
		else
		{
			Logger::perror("flock");
		}
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}

int
create_pid_file(const char* pid_file)
{
	int32_t	tm_pid;
	int		pid_fd;

	tm_pid = getpid();

	pid_fd = open(pid_file, O_RDWR | O_CREAT, TM_DEFAULT_FILE_MODE);
	if (pid_fd == -1)
	{
		Logger::perror("open");
		return (TM_FAILURE);
	}

	if (lock_pid(pid_fd) == TM_FAILURE)
	{
		close(pid_fd);
		return (TM_FAILURE);
	}

	ftruncate(pid_fd, 0);
	lseek(pid_fd, 0, SEEK_SET);
	dprintf(pid_fd, "%d\n", tm_pid);

	g_pid_fd = pid_fd;

	return (TM_SUCCESS);
}

void
remove_pid_file(const char* pid_file)
{
	if (g_pid_fd != -1)
	{
		flock(g_pid_fd, LOCK_UN);
		close(g_pid_fd);
		g_pid_fd = -1;
	}

	(void)unlink(pid_file);
}
