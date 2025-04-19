/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:26 by mgama             #+#    #+#             */
/*   Updated: 2025/04/19 11:01:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "tm.hpp"

enum tm_config_auto_restart {
	TM_CONF_AUTORESTART_FALSE		= 0,
	TM_CONF_AUTORESTART_UNEXPECTED	= 1,
	TM_CONF_AUTORESTART_TRUE		= 2,
};

enum tm_config_stop_signal {
	TERM	= SIGTERM,
	HUP		= SIGHUP,
	INT		= SIGINT,
	QUIT	= SIGQUIT,
	KILL	= SIGKILL,
	USR1	= SIGUSR1,
	USR2	= SIGUSR2,
};

typedef struct tm_process_config {
	/**
	 * Start the process automatically when the taskmaster starts.
	 * @default true
	 */
	bool					autostart;
	/**
	 * Automatically restart the process if it exits unexpectedly.
	 * @default unexpected
	 */
	tm_config_auto_restart	autorestart;
	/**
	 * List of exit codes that are considered successful.
	 */
	uint8_t					exitcodes[256];
	uint8_t					_exitcodes_len;
	/**
	 * Signal to send to the process when stopping it.
	 * @default TERM
	 */
	tm_config_stop_signal	stopsignal;
	/**
	 * Number of seconds to wait before considering the process started.
	 * @default 1
	 */
	int						startsecs;
	/**
	 * Number of retries to start the process if it fails to start.
	 * @default 3
	 */
	int						startretries;

	/**
	 * Constructor to initialize the process configuration with default values.
	 */
	tm_process_config(
		bool autostart = true,
		tm_config_auto_restart autorestart = TM_CONF_AUTORESTART_UNEXPECTED,
		const std::initializer_list<uint8_t> &init_exitcodes = { 0 },
		tm_config_stop_signal stopsignal = TERM,
		int startsecs = 1,
		int startretries = 3
	)
	{
		this->autostart = autostart;
		this->autorestart = autorestart;
		this->stopsignal = stopsignal;
		this->startsecs = startsecs;
		this->startretries = startretries;

		if (init_exitcodes.size() > 256)
		{
			throw std::out_of_range("Exit codes length exceeds maximum size of 256");
		}
		this->_exitcodes_len = init_exitcodes.size();
		size_t i = 0;
		for (auto code : init_exitcodes)
		{
			this->exitcodes[i++] = code;
		}
	}

	bool isExitCodeSuccessful(uint8_t exitcode) const
	{
		for (size_t i = 0; i < this->_exitcodes_len; ++i)
		{
			if (this->exitcodes[i] == exitcode)
			{
				return true;
			}
		}
		return false;
	}	
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

	bool	_stop_requested;

	time_point	start_time;
	time_point	stop_time;

	int		std_in_fd;
	int		std_out_fd;
	int		std_err_fd;

	tm_process_config	config; 
	int 				_retries;

	char* const*	exec;
	char* const*	envp;

	int		_wait(void);

	int		_spawn(void);
	int		_monitor_starting(void);
	int		_monitor_running(void);
	int		_monitor_stopping(void);

public:
	Process(char* const* exec, char* const* envp, pid_t ppid, int std_in_fd, int std_out_fd, int std_err_fd, tm_process_config &config);
	~Process(void);

	int		stop(void);
	int		kill(void);
	int		monitor(void);

	pid_t	getPid() const;

	bool	started(void) const;
	bool	stopped(void) const;
	bool	exited(void) const;
	bool	fatal(void) const;
	int		getStopSignal(void) const;
	bool	shouldRestart(void) const;
};

#endif /* PROCESS_HPP */