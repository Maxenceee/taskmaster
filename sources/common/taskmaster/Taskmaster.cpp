/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Taskmaster.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:40:49 by mgama             #+#    #+#             */
/*   Updated: 2025/01/18 20:01:30 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "taskmaster/Taskmaster.hpp"

Taskmaster::Taskmaster(char* const* envp)
{
	this->exit = false;

	this->envp = envp;
}

Taskmaster::~Taskmaster(void)
{
	
}

int	Taskmaster::addChild(char* const* exec)
{
	int	std_out_fd = open("child_stdout.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (std_out_fd == -1) {
		perror("Failed to open log file");
		return (1);
	}

	tm_process_config config = {
		.auto_restart = true
	};

	Process*	new_child = new Process(exec, -1, std_out_fd, -1, config);

	this->_processes.push_back(new_child);
	return (0);
}

int	Taskmaster::launch(void)
{
	for(const auto& process : this->_processes)
	{
		if (process->spawn(this->envp))
		{
			std::cout << "Could not spawn child" << std::endl;
		}
	}
	return (0);
}

int	Taskmaster::start(void)
{
	this->launch();

	do
	{
		for(const auto& process : this->_processes)
		{
			if (process->monitor())
			{
				break;
			}
		}
	} while (false == this->exit);
	
	return (0);
}