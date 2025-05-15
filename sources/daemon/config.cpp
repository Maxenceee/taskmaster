/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 07:59:30 by mgama             #+#    #+#             */
/*   Updated: 2025/05/15 16:12:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config.hpp"
#include "utils/utils.hpp"

std::ostream& operator<<(std::ostream& os, const tm_Config::UnixServer& server) {
	os << "UnixServer:\n";
	os << "  file: " << server.file << "\n";
	os << "  chmod: " << std::oct << std::setfill('0') << std::setw(3) << server.chmod << std::dec << "\n";
	os << "  chown: { uid: " << server.chown.uid << ", gid: " << server.chown.gid << " }\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const tm_Config::Daemon& daemon) {
	os << "Daemon:\n";
	os << "  logfile: " << daemon.logfile << "\n";
	os << "  pidfile: " << daemon.pidfile << "\n";
	os << "  logfile_maxbytes: " << daemon.logfile_maxbytes << "\n";
	os << "  umask: " << std::oct << std::setfill('0') << std::setw(3) << daemon.umask << std::dec << "\n";
	os << "  nodaemon: " << (daemon.nodaemon ? "true" : "false") << "\n";
	os << "  childlogdir: " << daemon.childlogdir << "\n";
	os << "  user: " << daemon.user << "\n";
	os << "  directory: " << daemon.directory << "\n";
	os << "  environment: [";
	for (auto it = daemon.environment.begin(); it != daemon.environment.end(); ++it) {
		os << *it;
		if (std::next(it) != daemon.environment.end()) {
			os << ", ";
		}
	}
	os << "]\n";
	return os;
}
std::ostream& operator<<(std::ostream& os, const tm_Config::Program& program) {
	os << "Program: " << program.name << "\n";
	os << "  raw_command: " << program.raw_command << "\n";
	os << "  command: [";
	for (auto it = program.command.begin(); it != program.command.end(); ++it) {
		os << *it;
		if (std::next(it) != program.command.end()) {
			os << ", ";
		}
	}
	os << "]\n";
	os << "  numprocs: " << program.numprocs << "\n";
	os << "  priority: " << program.priority << "\n";
	os << "  autostart: " << (program.autostart ? "true" : "false") << "\n";
	os << "  startsecs: " << program.startsecs << "\n";
	os << "  startretries: " << program.startretries << "\n";
	os << "  autorestart: " << program.autorestart << "\n";
	os << "  exitcodes: [";
	for (auto it = program.exitcodes.begin(); it != program.exitcodes.end(); ++it) {
		os << *it;
		if (std::next(it) != program.exitcodes.end()) {
			os << ", ";
		}
	}
	os << "]\n";
	os << "  stopsignal: " << getSignalName(program.stopsignal) << "\n";
	os << "  stopwaitsecs: " << program.stopwaitsecs << "\n";
	os << "  stopasgroup: " << (program.stopasgroup ? "true" : "false") << "\n";
	os << "  killasgroup: " << (program.killasgroup ? "true" : "false") << "\n";
	os << "  user: " << program.user << "\n";
	os << "  stdout_logfile: " << (program.stdout_logfile.length() ? program.stdout_logfile : "tempfile") << "\n";
	os << "  stderr_logfile: " << (program.stderr_logfile.length() ? program.stderr_logfile : "tempfile") << "\n";
	os << "  environment: [";
	for (auto it = program.environment.begin(); it != program.environment.end(); ++it) {
		os << *it;
		if (std::next(it) != program.environment.end()) {
			os << ", ";
		}
	}
	os << "]\n";
	os << "  directory: " << program.directory << "\n";
	os << "  umask: " << std::oct << std::setfill('0') << std::setw(3) << program.umask << std::dec << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const tm_Config& config) {
	os << config.server;
	os << config.daemon;
	os << "Programs:\n";
	for (auto it = config.programs.begin(); it != config.programs.end(); ++it) {
		os << *it;
	}
	return os;
}