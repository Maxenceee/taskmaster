/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:26 by mgama             #+#    #+#             */
/*   Updated: 2025/01/19 14:15:07 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "tm.hpp"

typedef struct tm_process_config {
	bool	auto_restart;
} tm_process_config;

enum tm_process_status {
	TM_P_IDLE		= 0,
	TM_P_STARTED	= 1,
	TM_P_EXITED		= 2,
};

class Process
{
private:
	pid_t	pid;
	int		_wpstatus;
	int		_signal;
	int		_exit_code;
	int		_status;

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

	bool	started(void) const;
	bool	exited(void) const;
};

#endif /* PROCESS_HPP */