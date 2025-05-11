/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:51 by mgama             #+#    #+#             */
/*   Updated: 2025/05/11 19:57:41 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TASKMASTER_HPP
#define TASKMASTER_HPP

#include "tm.hpp"
#include "process/Process.hpp"

#define TRUTHY_STRINGS(str) (str == "true" || str == "TRUE" || str == "True" || str == "1")
#define FALSY_STRINGS(str) (str == "false" || str == "FALSE" || str == "False" || str == "0")

struct tm_Config {
	struct UnixServer {
		std::string	file;
		uint16_t	chmod;
		uint32_t	chown[2]; // [uid, gid]
	} server;
	struct Daemon {
		std::string	logfile;
		std::string	pidfile;
		size_t		logfile_maxbytes;
		uint16_t	umask;
		bool		nodaemon;
		std::string	childlogdir;
		uid_t		user;
		std::string	directory;
		std::vector<std::string> environment;
	} daemon;
	struct Program {
		std::string name;
		std::string	command;
		std::string	process_name;
		uint16_t	numprocs;
		int			priority;
		bool		autostart;
		uint16_t	startsecs;
		uint16_t	startretries;
		tm_config_auto_restart	autorestart;
		std::vector<int>	exitcodes;
		unsigned	stopsignal;
		uint16_t	stopwaitsecs;
		bool		stopasgroup;
		bool		killasgroup;
		uid_t		user;
		std::string	stdout_logfile;
		std::string	stderr_logfile;
		std::vector<std::string> environment;
		std::string	directory;
		uint16_t	umask;
	};
	std::vector<Program> programs;
};

class Taskmaster
{
private:
	pid_t	pid;

	const std::string	_config_file;
	tm_Config			_config;

	std::vector<Process*>		_processes;

	bool	_has_prog(const std::string& progname) const;

public:
	Taskmaster(void);
	Taskmaster(const std::string& config_file);
	~Taskmaster(void);

	static bool	running;
	static bool	reload;

	int		addChild(char* const* exec, struct tm_process_config& config);
	int		start(void) const;
	int		restart(void) const;
	int		stop(void) const;
	int		signal(int sig) const;
	int		kill(void) const;
	int		cycle(void) const;

	int		readconfig(void);
	int		update(void);

	void	reopenStds(void) const;
	
	bool	allStopped(void) const;

	size_t		getNumProcesses(void) const;
	std::string	getDetailedStatus(void) const;
	std::string	getProcsStatus(void) const;

	const std::vector<Process*>&	all(void) const;
	Process*	find(const std::string& progname) const;
	Process*	get(uint16_t uid) const;
};

#endif /* TASKMASTER_HPP */