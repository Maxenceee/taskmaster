/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 19:44:23 by mgama             #+#    #+#             */
/*   Updated: 2025/04/20 12:11:02 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"

pid_t
Process::getPid(void) const
{
	return (this->pid);
}

bool
Process::started(void) const
{
	return (this->_state == TM_P_RUNNING);
}

bool
Process::stopped(void) const
{
	return (this->_state == TM_P_STOPPED);
}

bool
Process::exited(void) const
{
	return (this->_state == TM_P_EXITED);
}

bool
Process::fatal(void) const
{
	return (this->_state == TM_P_FATAL);
}

const std::string&
Process::getProgramName() const
{
	return (this->_program_name);
}

int
Process::getGroupId(void) const
{
	return (this->_process_group_id);
}

int
Process::getNumProcs(void) const
{
	return (this->config.numprocs);
}

int
Process::getStopSignal(void) const
{
	return (this->config.stopsignal);
}

int
Process::getSignal(void) const
{
	return (this->_signal);
}

int
Process::getExitCode(void) const
{
	return (this->_exit_code);
}

int
Process::getStdOutFd(void) const
{
	return (this->std_out_fd);
}

int
Process::getStdErrFd(void) const
{
	return (this->std_err_fd);
}

bool
Process::shouldRestart(void) const
{
	/**
	 * TODO:
	 * handler exit code check when TM_CONF_AUTORESTART_UNEXPECTED
	 */
	return (this->config.autorestart == TM_CONF_AUTORESTART_TRUE || this->config.autorestart == TM_CONF_AUTORESTART_UNEXPECTED);
}

std::string
Process::getState(void) const
{
	switch (this->_state)
	{
	case TM_P_STOPPED:
		return ("STOPPED");
	case TM_P_STARTING:
		return ("STARTING");
	case TM_P_RUNNING:
		return ("RUNNING");
	case TM_P_STOPPING:
		return ("STOPPING");
	case TM_P_EXITED:
		return ("EXITED");
	case TM_P_FATAL:
		return ("FATAL");
	default:
		return ("UNKNOWN");
	}
}

char const*
Process::getExecName(void) const
{
	return (this->exec[0]);
}

char* const*
Process::getExecArgs(void) const
{
	return (this->exec + 1);
}
