# Taskmaster

My **Taskmaster** projet for the 42 School cursus, a `[Supervisord](https://supervisord.org)` like process manager make in C++ 20.

## Getting started

You need to compile the project with `make`.

You will have two binaries, a daemon named `taskmasterd` and a client 'shell' named `taskmasterctl` to communicate with the daemon.

### Usage

**taskmasterd**: run a set of applications as daemons

```
Usage: ./taskmasterd [options]

Options:
  -c, --configuration <file> Specify configuration file
  -n, --nodaemon           Run in foreground (do not daemonize)
  -s, --silent             Suppress output to stdout/stderr
  -h, --help               Display this help and exit
  -v, --version            Display version information and exit
```

**taskmasterctl**: control applications run by taskmasterd from the cmd line

```
Usage: ./taskmasterctl [options] [action [arguments]]

Options:
  -i, --interactive        start an interactive shell after executing commands
  -h, --help               Display this help and exit

action [arguments] -- see below

Actions are commands like "tail" or "stop".  If -i is specified or no action is
specified on the command line, a "shell" interpreting actions typed
interactively is started.  Use the action "help" to find out about available
actions.
```

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
	- pidfile ✅
	- logfile_maxbytes ✅
	- umask ✅
	- user ✅
	- directory ✅
	- environment ✅

- program:*
	<!-- - process_name -->
	- numprocs ✅
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
	- environment ✅
	- directory ✅
	- umask ✅

## Features

### Table of contents
- [unix_server](#unix_server)
	- [file](#file)
	- [chmod](#chmod)
	- [chown](#chown)
- [taskmasterd](#taskmasterd)
	- [logfile](#logfile)
	- [pidfile](#pidfile)
	- [logfile_maxbytes](#logfile_maxbytes)
	- [umask](#umask)
	- [user](#user)
	- [directory](#directory)
	- [environment](#environment)
- [program:*](#program)
	- [numprocs](#numprocs)
	- [autostart](#autostart)
	- [startsecs](#startsecs)
	- [startretries](#startretries)
	- [autorestart](#autorestart)
	- [exitcodes](#exitcodes)
	- [stopsignal](#stopsignal)
	- [stopwaitsecs](#stopwaitsecs)
	- [stopasgroup](#stopasgroup)
	- [killasgroup](#killasgroup)
	- [user](#user-1)
	- [stdout_logfile](#stdout_logfile)
	- [stderr_logfile](#stderr_logfile)
	- [environment](#environment-1)
	- [directory](#directory-1)
	- [umask](#umask-1)

### Documentation

Based on Supervisord documentation ;)

#### `[unix_server]`

##### `file`

A path to a UNIX domain socket on which supervisor will listen for requests. taskmasterctl uses custom protocol to communicate with taskmasterd over this port.

*Default*: AUTO

*Required*: No

##### `chmod`

Change the UNIX permission mode bits of the UNIX domain socket to this value at startup.

*Default*: `0700`

*Required*: No

##### `chown`

Change the user and group of the socket file to this value. May be a UNIX username (e.g. `max`) or a UNIX username and group separated by a colon (e.g. `max:wheel`).

*Default*:  Use the username and group of the user who starts taskmasterd

*Required*: No

#### `[taskmasterd]`

##### `logfile`

The path to the activity log of the taskmasterd process.

*Default*: `$CWD/taskmasterd.log`

*Required*: No

##### `logfile_maxbytes`

The maximum number of bytes that may be consumed by the activity log file before it is rotated (suffix multipliers like “KB”, “MB”, and “GB” can be used in the value). Set this value to 0 to indicate an unlimited log size.

*Default*: `50MB`

*Required*: No

##### `pidfile`

The location in which taskmasterd keeps its pid file.

*Default*: `$CWD/taskmasterd.pid`

*Required*: No

##### `umask`

The `umask` of the taskmasterd process.

*Default*: `022`

*Required*: No

###### `user`

Instruct **taskmasterd** to switch users to this UNIX user account before doing any meaningful processing. The user can only be switched if **taskmasterd** is started as the root user.

*Default*: do not switch users

*Required*: No

##### `directory`

When **taskmasterd** daemonizes, switch to this directory.

*Default*: do not cd

*Required*: No

##### `environment`

A list of key/value pairs in the form `KEY="val",KEY2="val2"` that will be placed in the environment of all child processes. This does not change the environment of **taskmasterd** itself. Values containing non-alphanumeric characters should be quoted (e.g. `KEY="val:123",KEY2="val,456"`). Otherwise, quoting the values is optional but recommended.
Note that subprocesses will inherit the environment variables of the shell used to start **taskmasterd** except for the ones overridden here and within the program’s environment option

#### `[program]`

##### `command`

The command that will be run when this program is started. The command should be either absolute (e.g. `/path/to/programname`). Programs can accept arguments, e.g. `/path/to/program foo bar`. The command line can use double quotes to group arguments with spaces in them to pass to the program, e.g. `/path/to/program/name -p "foo bar"`. Controlled programs should themselves not be daemons, as **taskmasterd** assumes it is responsible for daemonizing its subprocesses.

*Default*: no values

*Required*: Yes

##### `numprocs`

Taskmaster will start as many instances of this program as named by numprocs.

*Default*: 1

*Required*: No

##### `autostart`

If true, this program will start automatically when **taskmasterd** is started.

*Default*: true

*Required*: No

##### `startsecs`

The total number of seconds which the program needs to stay running after a startup to consider the start successful (moving the process from the `STARTING` state to the `RUNNING` state). Set to 0 to indicate that the program needn’t stay running for any particular amount of time.

> Note:
>
> Even if a process exits with an “expected” exit code (see `exitcodes`), the start will still be considered a failure if the process exits quicker than `startsecs`.

*Default*: 1

*Required*: No

##### `startretries`

The number of serial failure attempts that **taskmasterd** will allow when attempting to start the program before giving up and putting the process into an `FATAL` state.

> Note:
>
> After each failed restart, process will be put in `BACKOFF` state and each retry attempt will take increasingly more time.

*Default*: 3

*Required*: No

##### `autorestart`

Specifies if **taskmasterd** should automatically restart a process if it exits when it is in the `RUNNING` state. May be one of `false`, `unexpected`, or `true`. If `false`, the process will not be autorestarted. If `unexpected`, the process will be restarted when the program exits with an exit code that is not one of the exit codes associated with this process’ configuration (see `exitcodes`). If `true`, the process will be unconditionally restarted when it exits, without regard to its exit code.

> Note:
>
> `autorestart` controls whether **taskmasterd** will autorestart a program if it exits after it has successfully started up (the process is in the `RUNNING` state).
>
> **taskmasterd** has a different restart mechanism for when the process is starting up (the process is in the `STARTING` state). Retries during process startup are controlled by `startsecs` and `startretries`.

*Default*: unexpected

*Required*: No

##### `exitcodes`

The list of “expected” exit codes for this program used with `autorestart`. If the `autorestart` parameter is set to `unexpected`, and the process exits in any other way than as a result of a taskmaster stop request, **taskmasterd** will restart the process if it exits with an exit code that is not defined in this list.

*Default*: 0

*Required*: No

##### `stopsignal`

The signal used to kill the program when a stop is requested. This can be specified using the signal’s name or its number. It is normally one of: `TERM`, `HUP`, `INT`, `QUIT`, `KILL`, `USR1`, or `USR2`.

*Default*: TERM

*Required*: No

##### `stopwaitsecs`

The number of seconds to wait for the child process to end after the program has been sent a stopsignal. If this number of seconds elapses, **taskmasterd** will attempt to kill it with a final SIGKILL.

*Default*: 10

*Required*: No

##### `stopasgroup`

If true, the flag causes supervisor to send the stop signal to the whole process group and implies `killasgroup` is true. This is useful for programs, such as Flask in debug mode, that do not propagate stop signals to their children, leaving them orphaned.

*Default*: false

*Required*: No

##### `killasgroup`

If true, when resorting to send SIGKILL to the program to terminate it send it to its whole process group instead, taking care of its children as well, useful e.g with Python programs using `multiprocessing`.

*Default*: false

*Required*: No

##### `user`

Instruct **taskmasterd** to use this UNIX user account as the account which runs the program. The user can only be switched if **taskmasterd** is run as the root user. If **taskmasterd** can’t switch to the specified user, the program will not be started.

*Default*: do not switch users

*Required*: No

##### `stdout_logfile`

Put process stdout output in this file. If `stdout_logfile` is unset, taskmaster will automatically choose a file location.

*Default*: auto

*Required*: No

##### `stderr_logfile`

Put process stderr output in this file. If `stderr_logfile` is unset, taskmaster will automatically choose a file location.

*Default*: auto

*Required*: No

##### `environment`

A list of key/value pairs in the form `KEY="val",KEY2="val2"` that will be placed in the environment of all child processes. This does not change the environment of **taskmasterd** itself. Values containing non-alphanumeric characters should be quoted (e.g. `KEY="val:123",KEY2="val,456"`). Otherwise, quoting the values is optional but recommended.
Note that subprocesses will inherit the environment variables of the shell used to start **taskmasterd** except for the ones overridden here and within the program’s environment option

*Default*: no extra environment

*Required*: No

##### `directory`

A file path representing a directory to which **taskmasterd** should temporarily chdir before exec’ing the child.

*Default*: No chdir (inherit taskmaster’s)

*Required*: No

##### `umask`

An octal number (e.g. 002, 022) representing the umask of the process.

*Default*: No special umask (inherit taskmaster’s)

*Required*: No