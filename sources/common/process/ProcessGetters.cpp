/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 19:44:23 by mgama             #+#    #+#             */
/*   Updated: 2025/03/22 12:23:46 by mgama            ###   ########.fr       */
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
	return (this->stop_sig);
}