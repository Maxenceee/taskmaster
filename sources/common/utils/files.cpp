/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   files.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/20 11:57:13 by mgama             #+#    #+#             */
/*   Updated: 2025/04/22 18:05:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

int
reopenstds(void)
{
	if (!isatty(STDOUT_FILENO))
	{
		int fd = open("/dev/tty", O_RDWR);
		if (fd == -1)
		{
			return (-1);
		}
		if (dup2(fd, STDOUT_FILENO) == -1)
		{
			close(fd);
			return (-1);
		}
	}
	if (!isatty(STDERR_FILENO))
	{
		int fd = open("/dev/tty", O_RDWR);
		if (fd == -1)
		{
			return (-1);
		}
		if (dup2(fd, STDERR_FILENO) == -1)
		{
			close(fd);
			return (-1);
		}
	}
	return (0);
}

int
tempfile(const char channel[3])
{
	// Attention : le template doit se terminer par "XXXXXX"
	char tmpname[64];

	snprintf(tmpname, sizeof(tmpname), "/tmp/taskmaster.child.%s.XXXXXX", channel);

	int fd = mkstemp(tmpname);
	if (fd == -1) {
		perror("mkstemp failed");
		return -1;
	}

	// Supprime le fichier du système de fichiers tout en gardant le fd ouvert
	// Cela garantit qu’il sera détruit quand le fd sera fermé
	unlink(tmpname);

	return fd;
}
