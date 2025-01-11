/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:13:48 by mgama             #+#    #+#             */
/*   Updated: 2025/01/11 19:47:47 by mgama            ###   ########.fr       */
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
#include <signal.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netdb.h>
#include <poll.h>
#include <arpa/inet.h>

#include <iostream>

#define TM_SOCKET_PATH "./taskmaster.sock"

#define TM_PIPE_READ_END 0
#define TM_PIPE_WRITE_END 1

// spawn
int	spawn_child(char *const *argv, char *const *envp, int stdin_fd, int stdout_fd, int stderr_fd);

typedef struct s_child_process {
	pid_t	pid;
	int		stdin_pipe_fd[2];
	int		stdout_pipe_fd[2];
	int		stderr_pipe_fd[2];
	int		exit_code;
	int 	status;
} tm_child_process_t;

struct s_taskmaster {
	tm_child_process_t *child;

	bool attach_output;
	int save_stdin;
	int save_stdout;
	int save_stderr;
};

#endif /* TASKMASTER_HPP */