/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/04/20 12:11:15 by mgama            ###   ########.fr       */
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

	char* const*					envp;

	// std::map<pid_t, Process*>		_processes;
	std::vector<Process*>		_processes;

public:
	Taskmaster(char* const* envp);
	~Taskmaster(void);

	static bool	running;

	int		addChild(char* const* exec);
	int		start(void);
	int		restart(void);
	int		stop(void);
	int		signal(int sig);
	int		kill(void);
	int		cycle(void);

	bool	allStopped(void) const;

	size_t		getNumProcesses(void) const;
	std::string	getStatus(void) const;
	Process*	getProcess(const std::string& progname, int replicas = 0) const;
};

#endif /* TASKMASTER_HPP */