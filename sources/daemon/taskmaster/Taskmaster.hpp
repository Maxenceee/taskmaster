/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/05/19 11:05:56 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TASKMASTER_HPP
#define TASKMASTER_HPP

#include "tm.hpp"
#include "config.hpp"
#include "process/Process.hpp"

class Taskmaster
{
private:
	pid_t	pid;

	const std::string	_config_file;
	tm_Config			_active_config;
	tm_Config			_read_config;

	std::vector<ProcessGroup*>	_processes;
	std::vector<ProcessGroup*>	_transitioning;
	// std::vector<Process*>		_unic_processes;

	void	_remove(const ProcessGroup *process);

public:
	explicit Taskmaster(void);
	explicit Taskmaster(const std::string& config_file);
	~Taskmaster(void);

	static bool	running;
	static bool	reload;

	int		start(void) const;
	int		restart(void) const;
	int		stop(void) const;
	int		signal(int sig) const;
	int		kill(void) const;
	int		cycle(void);

	int		readconfig(void);
	std::string	update(void);

	void	reopenStds(void) const;
	
	bool	allStopped(void) const;

	std::string	getProcsStatus(void) const;
	const tm_Config::UnixServer& getServerConf(void) const;
	std::string	getConfChanges(void) const;
	std::string	getAvailableProcs(void) const;

	const std::vector<Process*>	all(void) const;
	Process* const	find(const std::string& progname) const;
	Process* const	get(uint32_t uid) const;
};

#endif /* TASKMASTER_HPP */