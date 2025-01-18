/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:26 by mgama             #+#    #+#             */
/*   Updated: 2025/01/18 23:09:01 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "libs.hpp"

typedef struct tm_process_config {
	bool	auto_restart;
} tm_process_config;

class Process
{
private:
	pid_t	pid;
	int		_status;
	int		_signal;
	int		_exit_code;
	bool	_exited;

	time_point	start_time;
	time_point	stop_time;

	int		std_in_fd;
	int		std_out_fd;
	int		std_err_fd;

	bool	auto_restart;
	int		stop_sig;

	char* const*	exec;

public:
	Process(char* const* exec, int std_in_fd, int std_out_fd, int std_err_fd, tm_process_config &config);
	~Process(void);

	int		spawn(char* const* envp);
	int		stop(void);
	int		kill(void);
	int		monitor(void);

	pid_t	getPid() const;

	bool	exited(void) const;
};

#endif /* PROCESS_HPP */