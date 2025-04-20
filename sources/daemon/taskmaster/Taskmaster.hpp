/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/04/20 18:15:06 by mgama            ###   ########.fr       */
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

	std::vector<Process*>		_processes;

public:
	Taskmaster(char* const* envp);
	~Taskmaster(void);

	static bool	running;
	static bool	reload;

	int		addChild(char* const* exec);
	int		start(void) const;
	int		restart(void) const;
	int		stop(void) const;
	int		signal(int sig) const;
	int		kill(void) const;
	int		cycle(void) const;

	bool	allStopped(void) const;

	size_t		getNumProcesses(void) const;
	std::string	getStatus(void) const;
	Process*	find(const std::string& progname) const;
	Process*	get(const std::string& uid) const;
};

#endif /* TASKMASTER_HPP */