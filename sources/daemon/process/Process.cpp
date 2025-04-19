/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:28 by mgama             #+#    #+#             */
/*   Updated: 2025/04/19 13:17:02 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "spawn.hpp"
#include "utils/utils.hpp"
#include "logger/Logger.hpp"

Process::Process(char* const* exec, char* const* envp, pid_t ppid, int std_in_fd, int std_out_fd, int std_err_fd, tm_process_config &config)
{
	this->pid = -1;
	this->ppid = ppid;
	this->pgid = 0;
	this->_wpstatus = 0;
	this->_signal = 0;
	this->_exit_code = 0;
	this->_state = TM_P_STOPPED;
	this->_retries = 0;

	this->std_in_fd = std_in_fd;
	this->std_out_fd = std_out_fd;
	this->std_err_fd = std_err_fd;

	this->config = config;

	this->exec = exec;
	this->envp = envp;
}

Process::~Process(void)
{
	if (this->_state == TM_P_RUNNING && this->pid != -1)
	{
		std::cout << "Killing child " << this->pid << std::endl;
		(void)this->stop();
	}
}

int
Process::_spawn(void)
{
	if (this->_state == TM_P_RUNNING || this->_state == TM_P_STARTING || this->_state == TM_P_STOPPING)
	{
		Logger::error("Process already spawned");
		return (TM_FAILURE);
	}

	this->pid = -1;
	this->_signal = 0;
	this->_exit_code = 0;
	this->_state = TM_P_STARTING;
	this->_retries++;
	this->start_time = std::chrono::system_clock::now();

	if (access(exec[0], F_OK | X_OK) == -1)
	{
		Logger::error("requested executable does not exist or is not executable");
		return (TM_FAILURE);
	}

	if ((this->pid = spawn_child(this->exec, this->envp, this->std_in_fd, this->std_out_fd, this->std_err_fd, this->pgid)) == -1)
	{
		Logger::perror("could not spawn child");
		this->_state = TM_P_FATAL;
		return (TM_FAILURE);
	}

	std::cout << "Child spawned with pid " << this->pid << std::endl;

	return (TM_SUCCESS);
}

void
Process::setGroupId(pid_t pgid)
{
	if (pgid >= 0)
	{
		this->pgid = pgid;
	}
}

int
Process::start(void)
{
	if (this->_state == TM_P_RUNNING || this->_state == TM_P_STARTING || this->_state == TM_P_STOPPING)
	{
		std::cout << "Process already started" << std::endl;
		return (TM_FAILURE);
	}

	this->_retries = 0;
	return (this->_spawn());
}

int
Process::restart(void)
{
	this->waiting_restart = true;

	if (this->_state == TM_P_STOPPED || this->_state == TM_P_EXITED || this->_state == TM_P_FATAL)
	{
		std::cout << "Process already exited or in fatal state" << std::endl;
		this->_state = TM_P_EXITED;
		return (TM_SUCCESS);
	}

	return (this->stop());
}

int
Process::stop(void)
{
	if (this->_signal > 0 || this->_state == TM_P_STOPPING || this->_state == TM_P_EXITED || this->_state == TM_P_FATAL || this->pid == -1)
	{
		std::cout << "Process already stopping or stopped" << std::endl;
		return (TM_SUCCESS);
	}
	this->_state = TM_P_STOPPING;
	this->request_stop_time = std::chrono::steady_clock::now();

	std::cout << "Stopping child " << this->pid << " with signal " << strsignal(this->config.stopsignal) << std::endl;
	return (::kill(this->pid, this->config.stopsignal));
}

int
Process::signal(int sig)
{
	if (this->_state != TM_P_RUNNING || this->pid == -1)
	{
		std::cout << "Process already stopping or stopped" << std::endl;
		return (TM_SUCCESS);
	}
	std::cout << "Sending signal " << strsignal(sig) << " to child " << this->pid << std::endl;
	return (::kill(this->pid, sig));
}

int
Process::kill(void)
{
	if (this->_signal > 0 || this->_state == TM_P_EXITED || this->_state == TM_P_FATAL || this->pid == -1)
	{
		return (TM_SUCCESS);
	}
	this->_state = TM_P_EXITED;

	std::cout << "Killing child " << this->pid << std::endl;
	return (::kill(this->pid, SIGKILL));
}

int
Process::_wait(void)
{
	if (this->pid == -1)
	{
		return (TM_FAILURE);
	}

	if (waitpid(this->pid, &this->_wpstatus, WNOHANG) == this->pid)
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
		this->stop_time = std::chrono::system_clock::now();
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}

int
Process::monitor(void)
{
	switch (this->_state)
	{
	case TM_P_STOPPED:
		if (this->config.autostart)
		{
			return (this->_spawn());
		}
		break;
	case TM_P_STARTING:
	case TM_P_BACKOFF:
		return (this->_monitor_starting());
	case TM_P_RUNNING:
		return (this->_monitor_running());
	case TM_P_STOPPING:
		return (this->_monitor_stopping());
	case TM_P_EXITED:
		if (this->waiting_restart)
		{
			this->_retries = 0;
			this->waiting_restart = false;
			return (this->_spawn());
		}
		break;
	case TM_P_FATAL:
	case TM_P_UNKNOWN:
		break;
	}

	return (TM_SUCCESS);
}

int
Process::_monitor_starting(void)
{
	if (this->_state != TM_P_STARTING)
	{
		return (0);
	}

	if (this->_wait())
	{
		if (this->_retries < this->config.startretries)
		{
			this->_state = TM_P_BACKOFF;
			std::cout << "Child process " << this->pid << " failed to start" << std::endl;
			return (this->_spawn());
		}
		else
		{
			std::cout << "Child process " << this->pid << " failed to start, giving up" << std::endl;
			this->_state = TM_P_FATAL;
		}
		return (1);
	}
	else if (std::chrono::system_clock::now() - this->start_time > std::chrono::seconds(this->config.startsecs))
	{
		this->_state = TM_P_RUNNING;
		std::cout << "Child process " << this->pid << " started successfully" << std::endl;
		return (1);
	}

	return (0);
}

int
Process::_monitor_running(void)
{
	if (this->_wait())
	{
		this->_state = TM_P_EXITED;
		if (this->config.autorestart == TM_CONF_AUTORESTART_TRUE
			|| (this->config.autorestart == TM_CONF_AUTORESTART_UNEXPECTED && false == this->config.isExitCodeSuccessful(this->_exit_code))
			|| this->_signal != 0)
		{
			// this->_retries = 0;
			std::cout << "Child process " << this->pid << " exited, restarting" << std::endl;
			// return (this->_spawn());
			this->waiting_restart = true;
		}
		return (1);
	}

	return (0);
}

int
Process::_monitor_stopping(void)
{
	if (this->_wait())
	{
		this->_state = TM_P_EXITED;
		return (1);
	}
	else if (std::chrono::steady_clock::now() - this->request_stop_time > std::chrono::seconds(this->config.stopwaitsecs))
	{
		std::string sig = getSignalName(this->config.stopsignal);
		Logger::info("StopSignal " + sig + " failed to stop child in 10 seconds, resorting to SIGKILL");
		return (this->kill());
	}

	return (0);
}