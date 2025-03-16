/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:26 by mgama             #+#    #+#             */
/*   Updated: 2025/03/16 18:46:26 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "tm.hpp"

typedef struct tm_process_config {
	bool	auto_restart;
} tm_process_config;

enum tm_process_state {
	// The process has been stopped due to a stop request or has never been started.
	TM_P_STOPPED	= 0,
	// The process is starting due to a start request.
	TM_P_STARTING	= 10,
	// The process is running.
	TM_P_RUNNING	= 20,
	// The process entered the STARTING state but subsequently exited too quickly to move to the RUNNING state.
	TM_P_BACKOFF	= 30,
	// The process is stopping due to a stop request.
	TM_P_STOPPING	= 40,
	// The process exited from the RUNNING state (expectedly or unexpectedly).
	TM_P_EXITED		= 100,
	// The process could not be started successfully.
	TM_P_FATAL		= 200,
	// The process is in an unknown state.
	TM_P_UNKNOWN	= 1000,
};

class Process
{
private:
	pid_t	pid;
	pid_t	ppid;
	pid_t	pgid;
	int		_wpstatus;
	int		_signal;
	int		_exit_code;
	int		_state;

	time_point	start_time;
	time_point	stop_time;

	int		std_in_fd;
	int		std_out_fd;
	int		std_err_fd;

	bool	auto_restart;
	int		stop_sig;

	char* const*	exec;

public:
	Process(char* const* exec, pid_t ppid, int std_in_fd, int std_out_fd, int std_err_fd, tm_process_config &config);
	~Process(void);

	int		spawn(char* const* envp);
	int		stop(void);
	int		kill(void);
	int		monitor(void);

	pid_t	getPid() const;

	bool	started(void) const;
	bool	exited(void) const;
	bool	fatal(void) const;
};

#endif /* PROCESS_HPP */