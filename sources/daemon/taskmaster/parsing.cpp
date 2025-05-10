/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 07:59:30 by mgama             #+#    #+#             */
/*   Updated: 2025/05/10 14:08:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "inipp.hpp"
#include "taskmaster/Taskmaster.hpp"

enum tm_parse_type {
	TM_PARSE_TYPE_STRING	= 0x00,
	TM_PARSE_TYPE_INT		= 0x01,
	TM_PARSE_TYPE_BOOLEAN	= 0x02,
	TM_PARSE_TYPE_LIST		= 0x03,
};

const std::map<std::string, std::map<std::string, tm_parse_type>> Sections = {
	{"unix_server", {
		{"file", TM_PARSE_TYPE_STRING},
		{"chmod", TM_PARSE_TYPE_STRING},
		{"chown", TM_PARSE_TYPE_STRING},
	}},
	{"taskmasterd", {
		{"logfile", TM_PARSE_TYPE_STRING},
		{"pidfile", TM_PARSE_TYPE_STRING},
		{"logfile_maxbytes", TM_PARSE_TYPE_STRING},
		{"umask", TM_PARSE_TYPE_STRING},
		{"nodaemon", TM_PARSE_TYPE_BOOLEAN},
		{"childlogdir", TM_PARSE_TYPE_STRING},
		{"user", TM_PARSE_TYPE_STRING},
		{"directory", TM_PARSE_TYPE_STRING},
		{"environment", TM_PARSE_TYPE_LIST},
	}},
	{"program:", {
		{"command", TM_PARSE_TYPE_STRING},
		{"process_name", TM_PARSE_TYPE_STRING },
		{"numprocs", TM_PARSE_TYPE_INT},
		{"priority", TM_PARSE_TYPE_INT},
		{"autostart", TM_PARSE_TYPE_BOOLEAN},
		{"startsecs", TM_PARSE_TYPE_INT},
		{"startretries", TM_PARSE_TYPE_INT},
		{"autorestart", TM_PARSE_TYPE_STRING},
		{"exitcodes", TM_PARSE_TYPE_LIST},
		{"stopsignal", TM_PARSE_TYPE_STRING},
		{"stopwaitsecs", TM_PARSE_TYPE_INT},
		{"stopasgroup", TM_PARSE_TYPE_BOOLEAN},
		{"killasgroup", TM_PARSE_TYPE_BOOLEAN},
		{"user", TM_PARSE_TYPE_STRING},
		{"stdout_logfile", TM_PARSE_TYPE_STRING},
		{"stdout_syslog", TM_PARSE_TYPE_STRING},
		{"stderr_logfile", TM_PARSE_TYPE_STRING},
		{"environment", TM_PARSE_TYPE_LIST},
		{"directory", TM_PARSE_TYPE_STRING},
		{"umask", TM_PARSE_TYPE_STRING}
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
				continue;
			}
			std::cout << key.first << " = " << key.second << "\n";
		}
	}

	return (TM_SUCCESS);
}