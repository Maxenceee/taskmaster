/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:28 by mgama             #+#    #+#             */
/*   Updated: 2025/01/18 23:09:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "spawn.hpp"
#include "utils/utils.hpp"

Process::Process(char* const* exec, int std_in_fd, int std_out_fd, int std_err_fd, tm_process_config &config)
{
	this->pid = -1;
	this->_status = 0;
	this->_signal = 0;
	this->_exit_code = 0;
	this->_exited = false;

	this->std_in_fd = std_in_fd;
	this->std_out_fd = std_out_fd;
	this->std_err_fd = std_err_fd;

	this->auto_restart = config.auto_restart;
	this->stop_sig = SIGTERM;

	this->exec = exec;
}

Process::~Process(void)
{
	if (false == this->_exited && this->pid != -1)
	{
		std::cout << "Killing child " << this->pid << std::endl;
		this->stop();
	}
}

int	Process::spawn(char* const* envp)
{
	if ((this->pid = spawn_child(this->exec, envp, this->std_in_fd, this->std_out_fd, this->std_err_fd)) == -1)
	{
		return (1);
	}
	std::cout << "Child spawned with pid " << this->pid << std::endl;
	this->start_time = std::chrono::steady_clock::now();
	return (0);
}

int	Process::stop(void)
{
	if (this->_exited)
	{
		return (0);
	}
	return (::kill(this->pid, this->stop_sig));
}

int	Process::kill(void)
{
	if (this->_exited)
	{
		return (0);
	}
	return (::kill(this->pid, SIGKILL));
}

int	Process::monitor(void)
{
	if (waitpid(this->pid, &this->_status, WNOHANG) == this->pid)
	{
		if (WIFSIGNALED(this->_status))
		{
			this->_signal = WTERMSIG(this->_status);
			std::cout << "Child process " << this->pid << " terminated by signal " << strsignal(this->_signal) << std::endl;
		}
		else if (WIFEXITED(this->_status))
		{
			this->_exit_code = WEXITSTATUS(this->_status);
			std::cout << "Child process " << this->pid << " terminated with code " << this->_exit_code << std::endl;
		}
		this->stop_time = std::chrono::steady_clock::now();
		this->_exited = true;
		return (1);
	}
	return (0);
}