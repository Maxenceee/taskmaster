/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 07:59:30 by mgama             #+#    #+#             */
/*   Updated: 2025/11/12 11:08:57 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config.hpp"
#include "utils/utils.hpp"

template<typename T>
void
_print_list(std::ostream& os, const std::vector<T>& list, size_t padding = 0)
{
	os << "[";
	if (!list.empty())
	{
		os << "\n" << std::string(padding, ' ');
	}
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		os << "  " << *it;
		if (std::next(it) != list.end())
		{
			os << ", ";
		}
		os << "\n" << std::string(padding, ' ');
	}
	os << "]\n";
}

std::ostream& operator<<(std::ostream& os, const tm_Config::UnixServer& server)
{
	os << "UnixServer:\n";
	os << "  file: " << server.file << "\n";
	os << "  chmod: " << std::oct << std::setfill('0') << std::setw(3) << server.chmod << std::dec << "\n";
	os << "  chown: { uid: " << server.chown.uid << ", gid: " << server.chown.gid << " }\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const tm_Config::Daemon& daemon)
{
	os << "Daemon:\n";
	os << "  logfile: " << daemon.logfile << "\n";
	os << "  pidfile: " << daemon.pidfile << "\n";
	os << "  logfile_maxbytes: " << daemon.logfile_maxbytes << "\n";
	os << "  umask: " << std::oct << std::setfill('0') << std::setw(3) << daemon.umask << std::dec << "\n";
	os << "  nodaemon: " << (daemon.nodaemon ? "true" : "false") << "\n";
	os << "  user: " << daemon.user << "\n";
	os << "  directory: " << daemon.directory << "\n";
	os << "  environment: ";
	_print_list(os, daemon.environment, 2);
	return os;
}

std::ostream& operator<<(std::ostream& os, const tm_Config::Program& program)
{
	os << "Program: " << program.name << "\n";
	os << "  cid: " << program.cid << "\n";
	os << "  raw_command: " << program.raw_command << "\n";
	os << "  command: ";
	_print_list(os, program.command, 2);
	os << "  numprocs: " << program.numprocs << "\n";
	os << "  autostart: " << (program.autostart ? "true" : "false") << "\n";
	os << "  startsecs: " << program.startsecs << "\n";
	os << "  startretries: " << program.startretries << "\n";
	os << "  autorestart: " << program.autorestart << "\n";
	os << "  exitcodes: ";
	_print_list(os, program.exitcodes, 2);
	os << "  stopsignal: " << getSignalName(program.stopsignal) << "\n";
	os << "  stopwaitsecs: " << program.stopwaitsecs << "\n";
	os << "  stopasgroup: " << (program.stopasgroup ? "true" : "false") << "\n";
	os << "  killasgroup: " << (program.killasgroup ? "true" : "false") << "\n";
	os << "  user: " << program.user << "\n";
	os << "  stdout_logfile: " << (program.stdout_logfile.length() ? program.stdout_logfile : "(tmpfile)") << "\n";
	os << "  stderr_logfile: " << (program.stderr_logfile.length() ? program.stderr_logfile : "(tmpfile)") << "\n";
	os << "  environment: ";
	_print_list(os, program.environment, 2);
	os << "  directory: " << program.directory << "\n";
	os << "  umask: " << std::oct << std::setfill('0') << std::setw(3) << program.umask << std::dec << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const tm_Config& config)
{
	os << "Config ID: " << config.cid << "\n";
	os << config.server;
	os << config.daemon;
	os << "Programs:\n";
	for (auto it = config.programs.begin(); it != config.programs.end(); ++it)
	{
		os << *it;
	}
	return os;
}
