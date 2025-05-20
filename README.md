# Taskmaster

## Signal Handlers

- `SIGTERM`: taskmasterd and all its subprocesses will shut down. This may take several seconds.
- `SIGINT`: taskmasterd and all its subprocesses will shut down. This may take several seconds.
- `SIGQUIT`: taskmasterd and all its subprocesses will shut down. This may take several seconds.
- `SIGHUP`: taskmasterd will stop all processes, reload the configuration from the first config file it finds, and start all processes.
- `SIGUSR2`: taskmasterd will close and reopen the main activity log and all child log files.

## Configuration

- unix_server
	- file ✅
	- chmod ✅
	- chown ✅

- taskmasterd
	- logfile ✅
	- pidfile
	- logfile_maxbytes
	- umask
	- childlogdir
	- user
	- directory
	- environment

- program:*
	<!-- - process_name -->
	- numprocs ✅
	- priority
	- autostart ✅
	- startsecs ✅
	- startretries ✅
	- autorestart ✅
	- exitcodes ✅
	- stopsignal ✅
	- stopwaitsecs ✅
	- stopasgroup ✅
	- killasgroup ✅
	- user ✅
	- stdout_logfile ✅
	- stderr_logfile ✅
	- environment
	- directory ✅
	- umask