/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/04/18 19:00:57 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TASKMASTER_HPP
#define TASKMASTER_HPP

#include "tm.hpp"
#include "process/Process.hpp"

class Taskmaster
{
private:
	pid_t	pid;
	bool	exit;

	char* const*					envp;

	// std::map<pid_t, Process*>		_processes;
	std::vector<Process*>		_processes;

public:
	Taskmaster(char* const* envp);
	~Taskmaster(void);

	int		addChild(char* const* exec);
	int		stop(void);
	int		kill(void);
	int		cycle(void);

	bool	allStopped(void) const;
};

#endif /* TASKMASTER_HPP */