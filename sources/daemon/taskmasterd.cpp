/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/01/18 23:18:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libs.hpp"
#include "spawn.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

typedef struct s_child_process {
	pid_t	pid;
	int		stdin_pipe_fd[2];
	int		stdout_pipe_fd[2];
	int		stderr_pipe_fd[2];
	int		log_file_fd;
	int		exit_code;
	int 	status;
	int		signal;
	bool	auto_restart;
} tm_child_process_t;

struct s_taskmaster {
	tm_child_process_t *child;

	bool attach_output;
	int save_stdin;
	int save_stdout;
	int save_stderr;
};

struct s_taskmaster taskmaster = {
	.attach_output = false,
	.save_stdin = -1,
	.save_stdout = -1,
	.save_stderr = -1
};

int
create_child(tm_child_process_t *child, char* const* argv, char* const* envp)
{
	if (pipe(child->stdout_pipe_fd) == -1) {
		perror("pipe failed");
		return (1);
	}

	child->log_file_fd = open("child_stdout.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (child->log_file_fd == -1) {
		perror("Failed to open log file");
		return (1);
	}

	// write(child->log_file_fd, "Starting...\n", 12);

	// if ((child->pid = spawn_child(argv + 1, envp, -1, child->stdout_pipe_fd[TM_PIPE_WRITE_END], -1)) == -1) {
	if ((child->pid = spawn_child(argv + 1, envp, -1, child->log_file_fd, -1)) == -1) {
		return (1);
	}

	// if (dup2(child->log_file_fd, STDOUT_FILENO) == -1)
	// 	perror("dup2");

	// if (dup2(child->stdout_pipe_fd[TM_PIPE_READ_END], child->log_file_fd) == -1) {
	// 	perror("Failed to dup pipe to log file");
	// 	close(child->log_file_fd);
	// 	return 1;
	// }

	// write(child->log_file_fd, "after dup\n", 10);

	// close(child->stdout_pipe_fd[TM_PIPE_READ_END]);
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

bool Taskmaster::should_stop = false;

int
main(int argc, char* const* argv, char* const* envp)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <command> [args...]" << std::endl;
		return (1);
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	Logger::init();

	// read_config_file("/etc/taskmaster/taskmaster.conf");

	// tm_child_process_t child;

	// child.auto_restart = false;
	// taskmaster.child = &child;

	// if (create_child(&child, argv, envp) != 0) {
	// 	std::cerr << "Failed to create child process" << std::endl;
	// 	return (1);
	// }

	// std::cout << "Child started with pid " << child.pid << std::endl;

	// size_t i = 0;
	// do
	// {
	// 	int status;

	// 	if (waitpid(child.pid, &status, WNOHANG) == child.pid) {
	// 		child.status = WEXITSTATUS(status);

	// 		std::cout << "signaled: " << WIFSIGNALED(status) << " exited: " << WIFEXITED(status) << std::endl;

	// 		if (WIFSIGNALED(status)) {
	// 			child.signal = WTERMSIG(status);
    //     		std::cout << "Child process " << child.pid << " terminated by signal " << strsignal(child.signal) << std::endl;
	// 		} else {
	// 			std::cout << "Child process " << child.pid << " terminated with code " << child.status << std::endl;
	// 		}

	// 		if (child.auto_restart) {
	// 			if (create_child(&child, argv, envp) != 0) {
	// 				std::cerr << "Failed to restart child process" << std::endl;
	// 				break;
	// 			} else {
	// 				std::cout << "Child restarted with pid " << child.pid << std::endl;
	// 			}
	// 		}
	// 	}

	// } while (1);

	// kill(child.pid, SIGTERM);
	Taskmaster master(envp);

	master.addChild(argv + 1);
	master.start();
}
