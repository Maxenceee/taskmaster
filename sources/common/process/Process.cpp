/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:28 by mgama             #+#    #+#             */
/*   Updated: 2025/02/06 19:19:18 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "spawn.hpp"
#include "utils/utils.hpp"
#include "logger/Logger.hpp"

Process::Process(char* const* exec, int std_in_fd, int std_out_fd, int std_err_fd, tm_process_config &config)
{
	this->pid = -1;
	this->_wpstatus = 0;
	this->_signal = 0;
	this->_exit_code = 0;
	this->_status = TM_P_IDLE;

	this->std_in_fd = std_in_fd;
	this->std_out_fd = std_out_fd;
	this->std_err_fd = std_err_fd;

	this->auto_restart = config.auto_restart;
	this->stop_sig = SIGTERM;

	this->exec = exec;
}

Process::~Process(void)
{
	if (this->_status == TM_P_STARTED && this->pid != -1)
	{
		std::cout << "Killing child " << this->pid << std::endl;
		(void)this->stop();
	}
}

int
Process::spawn(char* const* envp)
{
	if (this->_status == TM_P_STARTED)
	{
		Logger::error("Process already spawned");
		return (TM_FAILURE);
	}

	if (access(exec[0], F_OK | X_OK) == -1)
	{
		Logger::perror("could not spawn child");
		return (TM_FAILURE);
	}

	if ((this->pid = spawn_child(this->exec, envp, this->std_in_fd, this->std_out_fd, this->std_err_fd)) == -1)
	{
		return (TM_FAILURE);
	}
	this->_status = TM_P_STARTED;
	std::cout << "Child spawned with pid " << this->pid << std::endl;
	this->start_time = std::chrono::steady_clock::now();
	
	return (TM_SUCCESS);
}

int
Process::stop(void)
{
	if (this->_status == TM_P_EXITED || this->pid == -1)
	{
		return (TM_SUCCESS);
	}
	std::cout << "Stopping child with signal " << strsignal(this->stop_sig) << std::endl;
	return (::kill(this->pid, this->stop_sig));
}

int
Process::kill(void)
{
	if (this->_status == TM_P_EXITED || this->pid == -1)
	{
		return (TM_SUCCESS);
	}
	return (::kill(this->pid, SIGKILL));
}

int
Process::monitor(void)
{
	if (this->_status == TM_P_EXITED || this->pid == -1)
		return (TM_FAILURE);

	if (this->_status == TM_P_STARTED && waitpid(this->pid, &this->_wpstatus, WNOHANG) == this->pid)
	{
		if (WIFSIGNALED(this->_wpstatus))
		{
			this->_signal = WTERMSIG(this->_wpstatus);
			std::cout << "Child process " << this->pid << " terminated by signal " << strsignal(this->_signal) << std::endl;
		}
		else if (WIFEXITED(this->_wpstatus))
		{
			this->_exit_code = WEXITSTATUS(this->_wpstatus);
			std::cout << "Child process " << this->pid << " terminated with code " << this->_exit_code << std::endl;
		}
		this->stop_time = std::chrono::steady_clock::now();
		this->_status = TM_P_EXITED;
		return (TM_FAILURE);
	}
	return (TM_SUCCESS);
}