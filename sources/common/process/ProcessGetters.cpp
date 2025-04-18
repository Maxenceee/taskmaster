/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 19:44:23 by mgama             #+#    #+#             */
/*   Updated: 2025/04/18 19:02:42 by mgama            ###   ########.fr       */
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

int
Process::getStopSignal(void) const
{
	return (this->config.stopsignal);
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