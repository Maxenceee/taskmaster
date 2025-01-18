/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:28 by mgama             #+#    #+#             */
/*   Updated: 2025/01/18 19:58:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "spawn.hpp"
#include "utils/utils.hpp"

Process::Process(char* const* exec, int std_in_fd, int std_out_fd, int std_err_fd, tm_process_config &config)
{
	this->pid = -1;
	this->status = 0;
	this->signal = 0;
	this->exit_code = 0;
	this->exited = false;

	this->std_in_fd = std_in_fd;
	this->std_out_fd = std_out_fd;
	this->std_err_fd = std_err_fd;

	this->auto_restart = config.auto_restart;

	this->exec = exec;
}

Process::~Process(void)
{
	if (false == this->exited)
	{
		kill(this->pid, SIGTERM);
	}
}

int	Process::spawn(char* const* envp)
{
	if ((this->pid = spawn_child(this->exec, envp, this->std_in_fd, this->std_out_fd, this->std_err_fd)) == -1)
	{
		return (1);
	}
	this->start_time = std::chrono::steady_clock::now();
	return (0);
}

int	Process::monitor(void)
{
	if (waitpid(this->pid, &this->status, WNOHANG) == this->pid)
	{
		if (WIFSIGNALED(this->status))
		{
			this->signal = WTERMSIG(this->status);
			std::cout << "Child process " << this->pid << " terminated by signal " << strsignal(this->signal) << std::endl;
		}
		else if (WIFEXITED(this->status))
		{
			this->exit_code = WEXITSTATUS(this->status);
			std::cout << "Child process " << this->pid << " terminated with code " << this->exit_code << std::endl;
		}
		this->exited = true;
		return (1);
	}
	return (0);
}