/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:45:26 by mgama             #+#    #+#             */
/*   Updated: 2025/04/24 11:29:21 by mgama            ###   ########.fr       */
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
	TM_S_TERM	= SIGTERM,
	TM_S_HUP	= SIGHUP,
	TM_S_INT	= SIGINT,
	TM_S_QUIT	= SIGQUIT,
	TM_S_KILL	= SIGKILL,
	TM_S_USR1	= SIGUSR1,
	TM_S_USR2	= SIGUSR2,
};

typedef struct tm_process_config {
	/**
	 * The number of repalicas to start for this process.
	 * @default 1
	 */
	uint16_t				numprocs;
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
	uint16_t				startsecs;
	/**
	 * Number of retries to start the process if it fails to start.
	 * @default 3
	 */
	uint16_t				startretries;
	/**
	 * Number of seconds to wait for the process to stop before forcefully killing it.
	 * @default 10
	 */
	uint16_t				stopwaitsecs;
	/**
	 * Working directory to chdir to before executing the process.
	 */
	char					directory[PATH_MAX + 1];
	/**
	 * File to log stdout output to.
	 */
	char					stdout_logfile[PATH_MAX + 1];
	/**
	 * File to log stderr output to.
	 */
	char					stderr_logfile[PATH_MAX + 1];

	/**
	 * Constructor to initialize the process configuration with default values.
	 */
	tm_process_config(
		uint16_t numprocs = 1,
		bool autostart = true,
		tm_config_auto_restart autorestart = TM_CONF_AUTORESTART_UNEXPECTED,
		const std::initializer_list<uint8_t> &init_exitcodes = { 0 },
		tm_config_stop_signal stopsignal = TM_S_TERM,
		uint16_t startsecs = 1,
		uint16_t startretries = 3,
		uint16_t stopwaitsecs = 10,
		const char* directory = nullptr,
		const char* stdout_logfile = nullptr,
		const char* stderr_logfile = nullptr
	)
	{
		this->numprocs = numprocs;
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

		this->stopwaitsecs = stopwaitsecs;
		if (directory != nullptr)
		{
			strncpy(this->directory, directory, PATH_MAX);
			this->directory[PATH_MAX] = '\0';
		}
		else
		{
			this->directory[0] = '\0';
		}

		if (stdout_logfile != nullptr)
		{
			strncpy(this->stdout_logfile, stdout_logfile, PATH_MAX);
			this->stdout_logfile[PATH_MAX] = '\0';
		}
		else
		{
			this->stdout_logfile[0] = '\0';
		}

		if (stderr_logfile != nullptr)
		{
			strncpy(this->stderr_logfile, stderr_logfile, PATH_MAX);
			this->stderr_logfile[PATH_MAX] = '\0';
		}
		else
		{
			this->stderr_logfile[0] = '\0';
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
	uint16_t	uid;

	pid_t	pid;
	pid_t	ppid;
	pid_t	pgid;
	int		_wpstatus;
	int		_signal;
	int		_exit_code;
	int		_state;
	int		_desired_state;

	const std::string	_program_name;
	int					_process_group_id;

	time_point	start_time;
	time_point	stop_time;

	time_point_steady	request_stop_time;
	bool				waiting_restart;

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

	int		_setupstds(void);

public:
	Process(char* const* exec, char* const* envp, const char* program_name, tm_process_config &config, pid_t ppid, pid_t pgid = 0);
	Process(char* const* exec, char* const* envp, const std::string& program_name, tm_process_config &config, pid_t ppid, pid_t pgid = 0);
	~Process(void);

	void	reopenStds(void);

	void	setGroupId(int id);

	int		start(void);
	int		restart(void);
	int		stop(void);
	int		signal(int sig);
	int		kill(void);
	int		monitor(void);

	pid_t		getPid() const;
	uint16_t	getUid(void) const;

	bool	operator==(uint16_t other) const;

	bool	started(void) const;
	bool	stopped(void) const;
	bool	exited(void) const;
	bool	fatal(void) const;

	time_duration	uptime(void) const;

	bool	reachedDesiredState(void) const;

	int		getStdOutFd(void) const;
	int		getStdErrFd(void) const;

	const std::string&	getProgramName(void) const;
	int			getGroupId(void) const;
	int			getNumProcs(void) const;
	int			getStopSignal(void) const;
	int			getSignal(void) const;
	int			getExitCode(void) const;
	int			getState(void) const;
	int			getDesiredState(void) const;

	static std::string	getStateName(int _state);

	std::string	getStatus(void) const;

	char const*		getExecName(void) const;
	char* const*	getExecArgs(void) const;
};

#endif /* PROCESS_HPP */