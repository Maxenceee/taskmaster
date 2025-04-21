/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 19:44:23 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 18:20:21 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"

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
	oss << "{\n";
	oss << "  PID: " << this->pid << ";\n";
	oss << "  State: " << Process::getStateName(this->_state) << ";\n";
	oss << "  Signal: " << this->_signal << ";\n";
	oss << "  Exit code: " << this->_exit_code << ";\n";
	oss << "  Program: " << this->getExecName() << ";\n";
	oss << "  ProgramArguments: (" << "\n";
	for (char* const* arg = this->getExecArgs(); *arg != nullptr; ++arg)
	{
		oss << "    - \"" << *arg << "\"\n";
	}
	oss << "  );\n";
	oss << "}\n";

	return (oss.str());
}