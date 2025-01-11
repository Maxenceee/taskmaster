/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spawn.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:15:30 by mgama             #+#    #+#             */
/*   Updated: 2025/01/11 21:12:29 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "taskmaster.hpp"

int
spawn_child(char *const *argv, char *const *envp, int stdin_fd, int stdout_fd, int stderr_fd)
{
	pid_t pid;
	int status;

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	if (stdin_fd != -1) {
		posix_spawn_file_actions_adddup2(&actions, stdin_fd, STDIN_FILENO);
		posix_spawn_file_actions_addclose(&actions, stdin_fd);
	}
	if (stdout_fd != -1) {
		posix_spawn_file_actions_adddup2(&actions, stdout_fd, STDOUT_FILENO);
		dprintf(STDERR_FILENO, "stdout_fd %d connected to STDOUT\n", stdout_fd);
		posix_spawn_file_actions_addclose(&actions, stdout_fd);
		setvbuf(stdout, NULL, _IONBF, 0);
	}
	if (stderr_fd != -1) {
		posix_spawn_file_actions_adddup2(&actions, stderr_fd, STDERR_FILENO);
		posix_spawn_file_actions_addclose(&actions, stderr_fd);
	}

	// Spawn the child process
	if (posix_spawn(&pid, argv[0], &actions, NULL, argv, envp) != 0) {
		perror("posix_spawn failed");
		posix_spawn_file_actions_destroy(&actions);
		return -1;
	}

	posix_spawn_file_actions_destroy(&actions);

	return pid;
}
