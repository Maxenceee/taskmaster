/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:28 by mgama             #+#    #+#             */
/*   Updated: 2025/05/11 12:33:30 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "spawn.hpp"
#include "utils/utils.hpp"
#include "logger/Logger.hpp"

Process::Process(char* const* exec, char* const* envp, const std::string& program_name, tm_process_config &config, pid_t ppid, pid_t pgid): Process(exec, envp, program_name.c_str(), config, ppid, pgid)
{
}

Process::Process(char* const* exec, char* const* envp, const char* program_name, tm_process_config &config, pid_t ppid, pid_t pgid): uid(u_uint16()), _program_name(program_name)
{
	this->pid = 0;
	this->ppid = ppid;
	this->pgid = pgid;
	this->_wpstatus = 0;
	this->_signal = 0;
	this->_exit_code = 0;
	this->_state = TM_P_STOPPED;
	this->_retries = 0;

	this->std_in_fd = -1;
	this->std_out_fd = -1;
	this->std_err_fd = -1;

	this->_process_group_id = 0; // 0 means it is the leader of the process group

	this->config = config;

	this->exec = exec;
	this->envp = envp;

	this->_setupstds();
}

Process::~Process(void)
{
	if (this->_state == TM_P_RUNNING && this->pid != 0)
	{
		Logger::error("Process destructor called while process is running.");
		(void)this->stop();
	}
}

void
Process::_setupstds(void)
{
	if (this->config.stdout_logfile[0] != '\0')
	{
		if ((this->std_out_fd = open(this->config.stdout_logfile, O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1)
		{
			Logger::perror("could not open stdout logfile");
		}
	}
	if (this->std_out_fd == -1)
	{
		this->std_out_fd = tempfile("out");
	}
	if (this->config.stderr_logfile[0] != '\0')
	{
		if ((this->std_err_fd = open(this->config.stderr_logfile, O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1)
		{
			Logger::perror("could not open stderr logfile");
		}
	}
	if (this->std_err_fd == -1)
	{
		this->std_err_fd = tempfile("out");
	}
}

void
Process::setGroupId(int id)
{
	this->_process_group_id = id;
}

void
Process::reopenStds(void)
{
	if (this->std_out_fd != -1)
	{
		(void)close(this->std_out_fd);
	}
	if (this->std_err_fd != -1)
	{
		(void)close(this->std_err_fd);
	}
	this->_setupstds();
}

int
Process::_spawn(void)
{
	if (this->_state == TM_P_RUNNING || this->_state == TM_P_STARTING || this->_state == TM_P_STOPPING)
	{
		Logger::error("Process already spawned");
		return (TM_FAILURE);
	}

	this->pid = 0;
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

	if ((this->pid = spawn_child(this->exec, this->envp, this->std_in_fd, this->std_out_fd, this->std_err_fd, this->config.directory)) == 0)
	{
		Logger::perror("could not spawn child");
		this->_state = TM_P_FATAL;
		return (TM_FAILURE);
	}

	Logger::info("spawned: " + this->_program_name + " with pid " + std::to_string(this->pid));

	return (TM_SUCCESS);
}

int
Process::start(void)
{
	if (this->_state == TM_P_RUNNING || this->_state == TM_P_STARTING || this->_state == TM_P_STOPPING)
	{
		return (TM_FAILURE);
	}
	this->_desired_state = TM_P_RUNNING;

	this->_retries = 0;
	return (this->_spawn());
}

int
Process::restart(void)
{
	this->waiting_restart = true;

	this->_desired_state = TM_P_RUNNING;

	if (this->_state == TM_P_STOPPED || this->_state == TM_P_EXITED || this->_state == TM_P_FATAL)
	{
		this->_state = TM_P_EXITED;
		return (TM_SUCCESS);
	}

	return (this->stop());
}

int
Process::stop(void)
{
	if (this->_signal > 0 || this->_state == TM_P_STOPPING || this->_state == TM_P_EXITED || this->_state == TM_P_FATAL || this->pid == 0)
	{
		return (TM_SUCCESS);
	}
	if (false == this->waiting_restart)
	{
		this->_desired_state = TM_P_EXITED;
	}
	this->_state = TM_P_STOPPING;
	this->request_stop_time = std::chrono::steady_clock::now();

	return (::kill(this->pid, this->config.stopsignal));
}

int
Process::signal(int sig)
{
	if ((this->_state != TM_P_RUNNING && this->_state != TM_P_STARTING && this->_state != TM_P_STOPPING) || this->pid == 0)
	{
		return (TM_SUCCESS);
	}
	return (::kill(this->pid, sig));
}

int
Process::kill(void)
{
	if (this->_signal > 0 || this->_state == TM_P_EXITED || this->_state == TM_P_FATAL || this->pid == 0)
	{
		return (TM_SUCCESS);
	}
	this->_state = TM_P_EXITED;
	if (false == this->waiting_restart)
	{
		this->_desired_state = TM_P_EXITED;
	}

	Logger::warning("Killing " + this->_program_name + " (" + std::to_string(this->pid) + ")" + " with SIGKILL");
	return (::kill(this->pid, SIGKILL));
}

int
Process::_wait(void)
{
	if (this->pid == 0)
	{
		return (TM_FAILURE);
	}

	if (waitpid(this->pid, &this->_wpstatus, WNOHANG) == this->pid)
	{
		if (WIFSIGNALED(this->_wpstatus))
		{
			this->_signal = WTERMSIG(this->_wpstatus);
		}
		else if (WIFEXITED(this->_wpstatus))
		{
			this->_exit_code = WEXITSTATUS(this->_wpstatus);
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
		this->_printStopInfo();
		if (this->_retries < this->config.startretries)
		{
			this->_state = TM_P_BACKOFF;
			return (this->_spawn());
		}
		else
		{
			this->_state = TM_P_FATAL;
			Logger::warning("gave up: " + this->_program_name + " entered FATAL state, too many start retries too quickly");
		}
		return (1);
	}
	else if (std::chrono::system_clock::now() - this->start_time > std::chrono::seconds(this->config.startsecs))
	{
		this->_state = TM_P_RUNNING;
		Logger::info("success: " + this->_program_name + " entered RUNNING state, process has stayed up for > than " + std::to_string(this->config.startsecs) + " seconds (startsecs)");
		return (1);
	}

	return (0);
}

int
Process::_monitor_running(void)
{
	if (this->_wait())
	{
		this->_printStopInfo();
		this->_state = TM_P_EXITED;
		if (this->config.autorestart == TM_CONF_AUTORESTART_TRUE
			|| (this->config.autorestart == TM_CONF_AUTORESTART_UNEXPECTED && false == this->config.isExitCodeSuccessful(this->_exit_code))
			|| this->_signal != 0)
		{
			this->_desired_state = TM_P_RUNNING;
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
		this->_printStopInfo();
		this->_state = TM_P_EXITED;
		return (1);
	}
	else if (std::chrono::steady_clock::now() - this->request_stop_time > std::chrono::seconds(this->config.stopwaitsecs))
	{
		std::string sig = getSignalName(this->config.stopsignal);
		Logger::info("StopSignal " + sig + " failed to stop child in " + std::to_string(this->config.stopwaitsecs) + " seconds, resorting to SIGKILL");
		return (this->kill());
	}

	return (0);
}

void
Process::_printStopInfo(void)
{
	std::ostringstream oss;
	oss << "exited: " << this->_program_name;
	oss << " (";
	if (this->_signal != 0)
	{
		oss << "terminated by " << getSignalName(this->_signal);
	}
	else
	{
		oss << "exit code " << this->_exit_code;
	}
	if (false == this->config.isExitCodeSuccessful(this->_exit_code) && this->_desired_state != TM_P_EXITED)
	{
		oss << "; not expected)";
		Logger::warning(oss.str());
	}
	else
	{
		oss << ")";
		Logger::info(oss.str());
	}
}