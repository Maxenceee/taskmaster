/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spawn.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:15:30 by mgama             #+#    #+#             */
/*   Updated: 2025/01/18 23:24:16 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"

#ifndef TM_SPAWN_CHILD_USE_FORK
int
spawn_child(char* const* argv, char* const* envp, int stdin_fd, int stdout_fd, int stderr_fd)
{
	pid_t pid;
	int status;

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	if (stdin_fd != -1) {
		posix_spawn_file_actions_adddup2(&actions, stdin_fd, STDIN_FILENO);
		// posix_spawn_file_actions_addclose(&actions, stdin_fd);
	}
	if (stdout_fd != -1) {
		posix_spawn_file_actions_adddup2(&actions, stdout_fd, STDOUT_FILENO);
		// posix_spawn_file_actions_addclose(&actions, stdout_fd);
		setvbuf(stdout, NULL, _IONBF, 0);
	}
	if (stderr_fd != -1) {
		posix_spawn_file_actions_adddup2(&actions, stderr_fd, STDERR_FILENO);
		// posix_spawn_file_actions_addclose(&actions, stderr_fd);
	}

	posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);

	sigset_t signal_set;
	sigemptyset(&signal_set);
	// Since the parent is handling these signals we need to reset their default behavior
	sigaddset(&signal_set, SIGINT);
	sigaddset(&signal_set, SIGQUIT);
	sigaddset(&signal_set, SIGTERM);

	posix_spawnattr_setsigmask(&attr, &signal_set);

	// Spawn the child process
	if (posix_spawn(&pid, argv[0], &actions, &attr, argv, envp) != 0) {
		perror("posix_spawn failed");
		posix_spawn_file_actions_destroy(&actions);
		posix_spawnattr_destroy(&attr);
		return -1;
	}

	posix_spawn_file_actions_destroy(&actions);
	posix_spawnattr_destroy(&attr);

	return pid;
}
#else
int
spawn_child(char* const* argv, char* const* envp, int stdin_fd, int stdout_fd, int stderr_fd)
{
	pid_t pid = fork();

	if (pid < 0) {
		// Fork failed
		perror("fork failed");
		return -1;
	}

	if (pid == 0) {
		// In child process

		// Since the parent is handling these signals we need to reset their default behavior
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);

		// Redirect stdin
		if (stdin_fd != -1) {
			if (dup2(stdin_fd, STDIN_FILENO) == -1) {
				perror("dup2 stdin failed");
				exit(EXIT_FAILURE);
			}
			// close(stdin_fd);
		}

		// Redirect stdout
		if (stdout_fd != -1) {
			if (dup2(stdout_fd, STDOUT_FILENO) == -1) {
				perror("dup2 stdout failed");
				exit(EXIT_FAILURE);
			}
			// close(stdout_fd);
		}

		// Redirect stderr
		if (stderr_fd != -1) {
			if (dup2(stderr_fd, STDERR_FILENO) == -1) {
				perror("dup2 stderr failed");
				exit(EXIT_FAILURE);
			}
			// close(stderr_fd);
		}

		// Execute the child process
		if (execve(argv[0], argv, envp) == -1) {
			perror("execve failed");
			exit(EXIT_FAILURE);
		}
	}

	// In parent process
	return pid;
}
#endif