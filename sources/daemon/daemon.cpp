/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 15:39:02 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 22:00:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "daemon.hpp"
#include "logger/Logger.hpp"
#include "taskmaster/Taskmaster.hpp"

extern Taskmaster*	g_master;

int // retourne 0 en cas de succès depuis l'enfant, > 0 en cas de succès depuis le parent, -1 en cas d'erreur
become_daemon(int flags)
{
	pid_t pid;
	/**
	 * Le premier fork va changer notre pid
	 * mais le sid et le pgid seront ceux
	 * du processus appelant.
	 */
	switch((pid = fork()))
	{
		case -1:
			Logger::perror("fork");
			return (-1);
		case 0: break;                  // l'enfant continue
		default: return (pid);
	}

	/**
	 * Exécutez le processus dans une nouvelle session sans terminal
	 * de contrôle. L'ID de groupe de processus sera l'ID du processus
	 * et donc, le processus sera le chef de groupe de processus.
	 * Après cet appel, le processus sera dans une nouvelle session,
	 * et il sera le chef de groupe de processus dans un nouveau
	 * groupe de processus.
	 */
	if(setsid() == -1) {               // devenir le leader de la nouvelle session
		Logger::perror("setsid");
		return (-1);
	}

	/**
	 * Nous allons forker à nouveau, également connu sous le nom de
	 * double fork. Ce deuxième fork rendra notre processus orphelin
	 * car le parent se terminera. Lorsque le processus parent se termine,
	 * le processus enfant sera adopté par le processus init avec l'ID 1.
	 * Le résultat de ce deuxième fork est un processus dont le parent
	 * est le processus init avec un ID de 1. Le processus sera dans sa
	 * propre session et groupe de processus et n'aura aucun terminal
	 * de contrôle. De plus, le processus ne sera pas le chef de groupe
	 * de processus et donc, ne pourra pas avoir de terminal de contrôle
	 * s'il y en avait un.
	 */
	switch((pid = fork()))
	{
		case -1:
			Logger::perror("fork");
			return (-1);
		case 0: break;                  // l'enfant sort continue
		default: return (pid);
	}

	if (0 == (flags & TM_NO_UMASK0))
		(void)umask(0);                 // effacer le masque de création de fichiers

	if (0 == (flags & TM_NO_CHDIR))
		(void)chdir("/");                // changer vers le répertoire racine

	if (flags & TM_CLOSE_FILES)          // fermer tous les fichiers ouverts
	{
		int dev_null_fd = open("/dev/null", O_RDWR);
		(void)dup2(dev_null_fd, STDIN_FILENO);    // Rediriger stdin
		(void)dup2(dev_null_fd, STDOUT_FILENO);   // Rediriger stdout
		(void)dup2(dev_null_fd, STDERR_FILENO);   // Rediriger stderr
		(void)close(dev_null_fd);                 // fermer le descripteur de fichier
	}

	return (TM_SUCCESS);
}
