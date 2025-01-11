/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spawn.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:15:30 by mgama             #+#    #+#             */
/*   Updated: 2025/01/11 13:15:40 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "taskmaster.hpp"

int
spawn_child(char *const *argv, char *const *envp, int redirect_fd = -1)
{
    pid_t pid;
    int status;

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);

    if (redirect_fd != -1) {
        // Redirect stdout of the child to the given file descriptor
        posix_spawn_file_actions_adddup2(&actions, redirect_fd, STDOUT_FILENO);
    }

    // Spawn the child process
    if (posix_spawn(&pid, argv[0], &actions, NULL, argv, envp) != 0) {
        perror("posix_spawn failed");
        posix_spawn_file_actions_destroy(&actions);
        return -1;
    }

    posix_spawn_file_actions_destroy(&actions);

    // Wait for the child process to finish
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
        return -1;
    }

    if (!WIFEXITED(status)) {
        printf("Child process did not terminate normally\n");
        return -1;
    }

    return WEXITSTATUS(status);
}
