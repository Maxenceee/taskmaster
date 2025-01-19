/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 19:44:23 by mgama             #+#    #+#             */
/*   Updated: 2025/01/19 14:15:03 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "process/Process.hpp"

pid_t	Process::getPid(void) const
{
	return (this->pid);
}

bool	Process::started(void) const
{
	return (this->_status == TM_P_STARTED);
}

bool	Process::exited(void) const
{
	return (this->_status == TM_P_EXITED);
}
