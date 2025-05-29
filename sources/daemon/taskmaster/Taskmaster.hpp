/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 19:26:37 by mgama            ###   ########.fr       */
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

	void	_remove(const ProcessGroup *process);

public:
	explicit Taskmaster(void);
	explicit Taskmaster(const std::string& config_file);
	~Taskmaster(void);

	static bool	running;
	static bool	reload;

	int		cycle(void);
	int		stop(void);

	int		add(const std::string& progname);
	int		remove(const std::string& progname);

	int		readconfig(void);
	std::string	update(void);

	void	reopenStds(void) const;
	
	bool	allStopped(void) const;

	const std::vector<ProcessGroup*>& getGroups(void) const;

	std::string	getProcsStatus(void) const;
	std::string	getAvailableProcs(void) const;

	const tm_Config::UnixServer& getServerConf(void) const;
	const tm_Config::Daemon& getDaemonConf(void) const;
	std::string	getConfChanges(void) const;

	const std::vector<Process*>	all(void) const;
	Process*	find(const std::string& progname) const;
	Process*	get(uint32_t uid) const;
};

#endif /* TASKMASTER_HPP */