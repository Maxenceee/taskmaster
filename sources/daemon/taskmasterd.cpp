/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/01/11 21:12:46 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "taskmaster.hpp"

// pid_t child_pid = -1;
// bool attach_output = false;

// void handle_sigusr1(int) {
//     // Attach child's output to parent's stdout
//     attach_output = true;
// }

// void handle_sigusr2(int) {
//     // Terminate the program
//     if (child_pid > 0) {
//         attach_output = false;
//     }
// }

// int
// main(int argc, char *const *argv, char *const *envp)
// {
//     // Set up signal handlers
//     signal(SIGUSR1, handle_sigusr1);
//     signal(SIGUSR2, handle_sigusr2);

//     // Start the child process
//     spawn_child(argv + 1, envp);

//     while (true) {
//         int pipe_fd[2];
//         if (attach_output) {
//             attach_output = false;
//             if (pipe(pipe_fd) == -1) {
//                 perror("pipe failed");
//                 continue;
//             }

//             // Spawn child with output redirection
//             if ((child_pid = fork()) == 0) {
//                 close(pipe_fd[0]); // Close read end
//                 spawn_child(argv, envp, pipe_fd[1]);
//                 close(pipe_fd[1]);
//                 exit(0);
//             } else {
//                 close(pipe_fd[1]); // Close write end
//                 char buffer[1024];
//                 ssize_t bytes_read;
//                 while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
//                     buffer[bytes_read] = '\0';
//                     std::cout << buffer;
//                 }
//                 close(pipe_fd[0]);
//             }
//         } else {
//             child_pid = fork();
//             if (child_pid == 0) {
// 				close(STDOUT_FILENO);
// 				close(STDERR_FILENO);
//                 spawn_child(argv, envp);
//                 exit(0);
//             }
//         }

//         // Parent waits for the child or handles signals
//         pause();
//     }

//     return 0;
// }

struct s_taskmaster taskmaster = {
	.attach_output = false,
	.save_stdin = -1,
	.save_stdout = -1,
	.save_stderr = -1
};

int
create_child(tm_child_process_t *child, char *const *argv, char *const *envp)
{
	if (pipe(child->stdout_pipe_fd) == -1) {
		perror("pipe failed");
		return (1);
	}

	if ((child->pid = spawn_child(argv + 1, envp, -1, child->stdout_pipe_fd[TM_PIPE_WRITE_END], -1)) == -1) {
		return (1);
	}
	close(child->stdout_pipe_fd[TM_PIPE_WRITE_END]);
	return (0);
}

int
attach_child(tm_child_process_t *child)
{
	if (!taskmaster.attach_output) {
		taskmaster.attach_output = true;

		int bytes_read;
		char buffer[1024];
		dprintf(STDERR_FILENO, "before read %d %d\n", child->stdout_pipe_fd[TM_PIPE_READ_END], child->stdout_pipe_fd[TM_PIPE_WRITE_END]);
		bytes_read = read(child->stdout_pipe_fd[TM_PIPE_READ_END], buffer, sizeof(buffer));
		if (bytes_read == 0) {
			dprintf(STDERR_FILENO, "No data available in pipe, child might have exited.\n");
			return 1;
		} else if (bytes_read == -1) {
			perror("read failed");
			return 1;
		}
		dprintf(STDERR_FILENO, "bytes_read %d\n", bytes_read);
		write(STDOUT_FILENO, buffer, bytes_read);
	}
	return (0);
}

int main(int argc, char *const *argv, char *const *envp)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <command> [args...]" << std::endl;
		return (1);
	}

	tm_child_process_t child;

	taskmaster.child = &child;

	if (create_child(&child, argv, envp) != 0) {
		std::cerr << "Failed to create child process" << std::endl;
		return (1);
	}

	// std::cout << "Child process " << child.pid << " started" << std::endl;

	// printf("before\n");
	// saved_output = dup(STDOUT_FILENO);
	// printf("atached saved %d\n", saved_output);
	// close(STDOUT_FILENO);

	// dprintf(STDERR_FILENO, "detached saved %d\n", saved_output);
	// dup2(saved_output, STDOUT_FILENO);
	// perror("dup2");
	// close(saved_output);
	// printf("after\n");

	size_t i = 0;
	do
	{
		printf("New life cycle %zu\n", i++);

		if (i == 5)
			attach_child(&child);
		if (i == 10)
			break;

		// if (waitpid(child.pid, &child.status, WNOHANG) == child.pid) {
		// 	child.status = WEXITSTATUS(child.status);

		// 	std::cout << "Child process " << child.pid << " terminated with code " << child.status << ", restarting..." << std::endl;

		// 	if (create_child(&child, argv, envp) != 0) {
		// 		std::cerr << "Failed to restart child process" << std::endl;
		// 		break;
		// 	} else {
		// 		std::cout << "Child process " << child.pid << " restarted" << std::endl;
		// 	}
		// }

		// Faire d'autres choses ou répondre aux commandes externes
		// std::cout << "Supervising child process " << child.pid << std::endl;

		sleep(1);
	} while (1);
}