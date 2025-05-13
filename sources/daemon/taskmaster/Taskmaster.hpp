/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/05/13 19:44:20 by mgama            ###   ########.fr       */
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
	// std::vector<Process*>		_unic_processes;

	bool	_has_prog(const std::string& progname) const;

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
	int		cycle(void) const;

	int		readconfig(void);
	void	update(void);

	void	reopenStds(void) const;
	
	bool	allStopped(void) const;

	std::string	getProcsStatus(void) const;
	const tm_Config::UnixServer& getServerConf(void) const;

	const std::vector<Process*>	all(void) const;
	Process*	find(const std::string& progname) const;
	Process*	get(uint16_t uid) const;
};

#endif /* TASKMASTER_HPP */