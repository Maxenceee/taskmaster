/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 09:47:45 by mgama             #+#    #+#             */
/*   Updated: 2025/05/30 18:04:35 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "tm.hpp"

#define TRUTHY_STRINGS(str) (str == "true" || str == "TRUE" || str == "True" || str == "1")
#define FALSY_STRINGS(str) (str == "false" || str == "FALSE" || str == "False" || str == "0")

enum tm_config_auto_restart {
	TM_CONF_AUTORESTART_FALSE		= 0,
	TM_CONF_AUTORESTART_UNEXPECTED	= 1,
	TM_CONF_AUTORESTART_TRUE		= 2,
};

enum tm_config_stop_signal {
	TM_S_TERM	= SIGTERM,
	TM_S_HUP	= SIGHUP,
	TM_S_INT	= SIGINT,
	TM_S_QUIT	= SIGQUIT,
	TM_S_KILL	= SIGKILL,
	TM_S_USR1	= SIGUSR1,
	TM_S_USR2	= SIGUSR2,
};

struct tm_Config {
	struct UnixServer {
		std::string	file;
		uint16_t	chmod;
		struct owner {
			pid_t	uid;
			gid_t	gid;
		} chown;
	} server;
	struct Daemon {
		std::string	logfile;
		std::string	pidfile;
		size_t		logfile_maxbytes;
		mode_t		umask;
		bool		nodaemon;
		std::string	childlogdir;
		uid_t		user;
		std::string	directory;
		std::vector<std::string> environment;
	} daemon;
	struct Program {
		std::string name;
		std::string	raw_command;
		std::vector<std::string>	command;
		uint16_t	numprocs;
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
		mode_t		umask;
	};
	std::vector<Program> programs;
};

std::ostream&	operator<<(std::ostream& os, const tm_Config::UnixServer& server);
std::ostream&	operator<<(std::ostream& os, const tm_Config::Daemon& daemon);
std::ostream&	operator<<(std::ostream& os, const tm_Config::Program& program);
std::ostream&	operator<<(std::ostream& os, const tm_Config& config);

#endif /* CONFIG_HPP */