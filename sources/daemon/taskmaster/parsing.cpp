/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 07:59:30 by mgama             #+#    #+#             */
/*   Updated: 2025/05/08 08:46:53 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "inipp.hpp"
#include "taskmaster/Taskmaster.hpp"

const std::map<std::string, std::map<std::string, std::string>> Sections = {
	{"unix_server", {
		{"file", ""},
		{"chmod", ""},
		{"chown", ""},
	}},
	{"taskmasterd", {
		{"logfile", ""},
		{"pidfile", ""},
		{"logfile_maxbytes", ""},
		{"umask", ""},
		{"nodaemon", ""},
		{"childlogdir", ""},
		{"user", ""},
		{"directory", ""},
		{"environment", ""},
	}},
	{"program:", {
		{"process_name", ""},
		{"numprocs", ""},
		{"priority", ""},
		{"autostart", ""},
		{"startsecs", ""},
		{"startretries", ""},
		{"autorestart", ""},
		{"exitcodes", ""},
		{"stopsignal", ""},
		{"stopwaitsecs", ""},
		{"stopasgroup", ""},
		{"killasgroup", ""},
		{"user", ""},
		{"stdout_logfile", ""},
		{"stdout_syslog", ""},
		{"stderr_logfile", ""},
		{"environment", ""},
		{"directory", ""},
		{"umask", ""}
	}},
};

int
Taskmaster::parseConfig(const std::string& filename)
{
	inipp::Ini<char> ini;

	std::ifstream is(filename);
	ini.parse(is);
	for (auto section : ini.sections)
	{
		if (Sections.count(section.first) == 0)
		{
			std::cerr << "Unknown section: " << section.first << "\n";
			continue;
		}
		std::cout << "[" << section.first << "]" << "\n";
		for (auto key : section.second)
		{
			if (section.second.count(key.first) == 0)
			{
				std::cerr << "Unknown key: " << key.first << "\n";
			}
			std::cout << key.first << " = " << key.second << "\n";
		}
	}

	return (TM_SUCCESS);
}