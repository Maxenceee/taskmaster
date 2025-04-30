/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 19:44:23 by mgama             #+#    #+#             */
/*   Updated: 2025/04/30 23:19:30 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"
#include "utils/utils.hpp"

pid_t
Process::getPid(void) const
{
	return (this->pid);
}

uint16_t
Process::getUid(void) const
{
	return (this->uid);
}

bool
Process::operator==(uint16_t other) const
{
	return (this->uid == other);
}

bool
Process::operator==(const std::string& other) const
{
	return (this->_program_name == other);
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

time_duration
Process::uptime(void) const
{
	switch (this->_state)
	{	
	case TM_P_STARTING:
	case TM_P_RUNNING:
	case TM_P_BACKOFF:
	case TM_P_STOPPING:
		return (std::chrono::system_clock::now() - this->start_time);

	case TM_P_STOPPED:
	case TM_P_EXITED:
	case TM_P_FATAL:
	case TM_P_UNKNOWN:
	default:
		return (this->stop_time - this->start_time);
	}
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
Process::reachedDesiredState(void) const
{
	return (this->_state == this->_desired_state);
}

int
Process::getState(void) const
{
	return (this->_state);
}

int
Process::getDesiredState(void) const
{
	return (this->_desired_state);
}

std::string
Process::getStateName(int _state)
{
	switch (_state)
	{
	case TM_P_STOPPED:
		return ("STOPPED");
	case TM_P_STARTING:
		return ("STARTING");
	case TM_P_RUNNING:
		return ("RUNNING");
	case TM_P_BACKOFF:
		return ("BACKOFF");
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

std::string
Process::getStatus(void) const
{
	std::ostringstream oss;
	oss << std::setw(30) << std::left << this->getProgramName();
	oss << std::setw(9) << Process::getStateName(this->_state) << " ";
	switch (this->_state)
	{
	case TM_P_STOPPED:
		oss << "Not started";
		break;
	case TM_P_STARTING:
	case TM_P_BACKOFF:
	case TM_P_STOPPING:
		break;
	case TM_P_RUNNING:
		oss << "pid " << this->getPid() << ", ";
		oss << "uptime " << format_duration(this->uptime());
		break;
	case TM_P_EXITED:
		auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);

		oss << std::put_time(std::localtime(&time), "%b %d %I:%M %p");
		break;
	}
	oss << "\n";

	return (oss.str());
}