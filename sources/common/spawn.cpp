/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spawn.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:15:30 by mgama             #+#    #+#             */
/*   Updated: 2025/05/30 16:13:44 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "logger/Logger.hpp"

int
spawn_child(char* const* argv, char* const* envp, int stdin_fd, int stdout_fd, int stderr_fd
#ifdef TM_SPAWN_CHILD_USE_FORK
	, uid_t uid
#endif /* TM_SPAWN_CHILD_USE_FORK */
	, gid_t pgid, const char* dir, mode_t mode)
{
	pid_t pid;

#ifndef TM_SPAWN_CHILD_USE_FORK

	posix_spawn_file_actions_t actions;
	(void)posix_spawn_file_actions_init(&actions);

	if (stdin_fd != -1)
	{
		(void)posix_spawn_file_actions_adddup2(&actions, stdin_fd, STDIN_FILENO);
	}
	if (stdout_fd != -1)
	{
		(void)posix_spawn_file_actions_adddup2(&actions, stdout_fd, STDOUT_FILENO);
	}
	if (stderr_fd != -1)
	{
		(void)posix_spawn_file_actions_adddup2(&actions, stderr_fd, STDERR_FILENO);
	}

	posix_spawnattr_t attr;
	(void)posix_spawnattr_init(&attr);

	sigset_t signal_set;
	sigemptyset(&signal_set);
	// Since the parent is handling these signals we need to reset their default behavior
	sigaddset(&signal_set, SIGINT);
	sigaddset(&signal_set, SIGQUIT);
	sigaddset(&signal_set, SIGTERM);
	sigaddset(&signal_set, SIGPIPE);

	(void)posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK);
	if (posix_spawnattr_setsigmask(&attr, &signal_set) != 0)
	{
		Logger::perror("posix_spawnattr_setsigmask failed");
		(void)posix_spawn_file_actions_destroy(&actions);
		(void)posix_spawnattr_destroy(&attr);
		return (-1);
	}

	(void)posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
	if (posix_spawnattr_setpgroup(&attr, pgid) != 0)
	{
		Logger::perror("posix_spawnattr_setpgroup failed");
		(void)posix_spawn_file_actions_destroy(&actions);
		(void)posix_spawnattr_destroy(&attr);
		return (-1);
	}

	if (dir) {
		// Vérifie si le dossier existe avant
		if (access(dir, X_OK) != 0) {
			Logger::perror("invalid working directory");
			(void)posix_spawn_file_actions_destroy(&actions);
			(void)posix_spawnattr_destroy(&attr);
			return (-1);
		}

		if (posix_spawn_file_actions_addchdir_np(&actions, dir) != 0) {
			Logger::perror("posix_spawn_file_actions_addchdir_np failed");
			(void)posix_spawn_file_actions_destroy(&actions);
			(void)posix_spawnattr_destroy(&attr);
			return (-1);
		}
	}

	// Since posix_spawn doesn't support umask, we need to set it manually
	// to save the old one and restore it after the spawn
	mode_t mask = umask(mode);

	// Spawn the child process
	if (posix_spawn(&pid, argv[0], &actions, &attr, argv, envp) != 0)
	{
		Logger::perror("posix_spawn failed");
		(void)posix_spawn_file_actions_destroy(&actions);
		(void)posix_spawnattr_destroy(&attr);
		(void)umask(mask);
		return (-1);
	}

	(void)posix_spawn_file_actions_destroy(&actions);
	(void)posix_spawnattr_destroy(&attr);
	(void)umask(mask);

#else

	if ((pid = fork()) < 0) {
		Logger::perror("fork");
		return (-1);
	}

	if (pid == 0) {
		// In child process

		// Redirect stdin
		if (stdin_fd != -1 && dup2(stdin_fd, STDIN_FILENO) == -1) {
			Logger::perror("dup2 stdin failed");
			exit(TM_FAILURE);
		}

		// Redirect stdout
		if (stdout_fd != -1 && dup2(stdout_fd, STDOUT_FILENO) == -1) {
			Logger::perror("dup2 stdout failed");
			exit(TM_FAILURE);
		}

		// Redirect stderr
		if (stderr_fd != -1 && dup2(stderr_fd, STDERR_FILENO) == -1) {
			Logger::perror("dup2 stderr failed");
			exit(TM_FAILURE);
		}

		// Since the parent is handling these signals we need to reset their default behavior
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGPIPE, SIG_DFL);

		if (uid != -1 && setuid(uid) == -1) {
			Logger::perror("setuid");
			exit(TM_FAILURE);
		}

		if (setpgid(0, pgid) == -1) {
			Logger::perror("setpgid");
			exit(TM_FAILURE);
		}

		if (dir) {
			if (access(dir, X_OK) != 0) {
				Logger::perror("invalid working directory");
				exit(TM_FAILURE);
			}

			if (chdir(dir) != 0) {
				Logger::perror("chdir failed");
				exit(TM_FAILURE);
			}
		}

		(void)umask(mode);

		// Execute the child process
		if (execve(argv[0], argv, envp) == -1) {
			Logger::perror("execve");
			exit(TM_FAILURE);
		}
	}

#endif

	return pid;
}
