/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/20 13:59:14 by mgama             #+#    #+#             */
/*   Updated: 2025/04/20 14:09:05 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"

struct tm_internal_command
{
	// The command to execute
	std::string					command;
	// The target process name
	std::string					progname;
	// The target process group id (-1 for all)
	int							replica_id;
	// The command arguments
	std::vector<std::string>	args;
};

/**
 * The internal communication protocol is a simple text-based protocol
 * 
 * Each part of the command line is separated by a CRLF (\r\n) to ensure
 * proper parsing.
 * 
 * Example:
 * status\r\nfoo:1
 * 
 */

struct tm_internal_command	parse_internal(const char* cmd)
{
	struct tm_internal_command	cmd_struct;
}