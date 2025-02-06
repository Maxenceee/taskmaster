/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/02/06 19:06:21 by mgama            ###   ########.fr       */
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

	int		launch(void);

public:
	Taskmaster(char* const* envp);
	~Taskmaster(void);

	int		addChild(char* const* exec);

	int		start(void);
	int		stop(void);
	int		kill(void);
	int		cycle(void);

	bool	allStopped(void) const;
};

#endif /* TASKMASTER_HPP */