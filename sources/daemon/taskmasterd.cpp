/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/01/31 23:55:16 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "spawn.hpp"
#include "signal.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"

int
main(int argc, char* const* argv, char* const* envp)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <command> [args...]" << std::endl;
		return (1);
	}

	setup_signal(SIGINT, SIG_IGN);
	setup_signal(SIGQUIT, SIG_IGN);
	setup_signal(SIGTERM, SIG_IGN);

	Logger::init();

	Taskmaster master(envp);

	master.addChild(argv + 1);
	master.start();
}
