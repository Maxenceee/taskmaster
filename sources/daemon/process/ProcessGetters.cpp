/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 19:44:23 by mgama             #+#    #+#             */
/*   Updated: 2025/05/30 16:09:40 by mgama            ###   ########.fr       */
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

uint32_t
Process::getPuid(void) const
{
	return (TM_P_UID(this->gid, this->uid));
}

bool
Process::operator==(uint16_t other) const
{
	return (this->uid == other);
}

bool
Process::operator==(const std::string& other) const
{
	return (this->_process_name == other);
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
	return (this->_state == TM_P_FATAL || this->_state == TM_P_UNKNOWN);
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
Process::getProcessName() const
{
	return (this->_process_name);
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

std::string
Process::getStatus(void) const
{
	std::ostringstream oss;
	oss << std::setw(30) << std::left << this->getProcessName() << " ";
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

std::ostream&
operator<<(std::ostream& os, const Process& proc)
{
	os << proc.getStatus();
	return os;
}

std::ostream&
Process::operator<<(std::ostream& os) const
{
	os << this->getStatus();
	return os;
}
