/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 22:03:18 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "spawn.hpp"
#include "signal.hpp"
#include "pid.hpp"
#include "getopt.hpp"
#include "daemon.hpp"
#include "taskmaster/Taskmaster.hpp"
#include "logger/Logger.hpp"
#include "server/UnixSocketServer.hpp"

bool	Taskmaster::running = false;
bool	Taskmaster::reload = false;
bool	deamonized = false;

Taskmaster* g_master = nullptr;

static void
usage(char const* exec)
{
	std::cout << TM_PROJECTD " -- run a set of applications as daemons" << "\n\n";
	std::cout << "Usage: " << exec << " [options]" << "\n\n";
	std::cout << "Options:" << "\n";
	std::cout << "  " << "-c" << ", " << std::left << std::setw(20) << "--configuration <file>" << " Specify configuration file" << "\n";
	std::cout << "  " << "-n" << ", " << std::left << std::setw(20) << "--nodaemon" << " Run in foreground (do not daemonize)" << "\n";
	std::cout << "  " << "-s" << ", " << std::left << std::setw(20) << "--silent" << " Suppress output to stdout/stderr" << "\n";
	std::cout << "  " << "-h" << ", " << std::left << std::setw(20) << "--help" << " Display this help and exit" << "\n";
	std::cout << "  " << "-v" << ", " << std::left << std::setw(20) << "--version" << " Display version information and exit" << "\n";
	exit(64);
}

static void
interruptHandler(int sig_int)
{
	Logger::syn("\b\b"); // rm ^C from tty
	Logger::print("Signal received: " + std::string(strsignal(sig_int)), B_GREEN);
	if (Taskmaster::running == false)
	{
		Logger::info(TM_PROJECTD " is already stopping");
	}
	Taskmaster::running = false;
}

static void
interruptReopen(int sig_int)
{
	(void)sig_int;
	if (g_master == nullptr)
	{
		throw std::logic_error("This signal handler should not be bound before the main loop");
	}
	g_master->reopenStds();
	Logger::info("Reopening log files");
}

static void
interruptReload(int sig_int)
{
	(void)sig_int;
	Logger::print("Reloading the daemon...");
	Taskmaster::running = false;
	Taskmaster::reload = true;
}

inline static void
setup_signals(void)
{
	setup_signal(SIGINT, interruptHandler);
	setup_signal(SIGQUIT, interruptHandler);
	setup_signal(SIGTERM, interruptHandler);
	setup_signal(SIGHUP, interruptReload);
	setup_signal(SIGUSR2, interruptReopen);
}

inline static void
ignore_signals(void)
{
	setup_signal(SIGINT, SIG_IGN);
	setup_signal(SIGQUIT, SIG_IGN);
	setup_signal(SIGTERM, SIG_IGN);
	setup_signal(SIGHUP, SIG_IGN);
	setup_signal(SIGUSR2, SIG_IGN);
}

inline static void
start_main_loop(const std::string& config_file)
{
	Taskmaster master(config_file);
	g_master = &master;

	if (master.readconfig() == TM_FAILURE)
	{
		if (deamonized)
		{
			Logger::error("An error occurred while reading the configuration file, using previous configuration.");
		}
		else
		{
			throw std::runtime_error("Could not start the daemon.");
		}
	}
	master.update();

	auto pidfile = master.getDaemonConf().pidfile;

	if (create_pid_file(pidfile.c_str()) == TM_FAILURE)
	{
		throw std::runtime_error("Could not start the daemon.");
	}

	UnixSocketServer server(master.getServerConf().file.c_str(), master);

	if (false == deamonized)
	{
		pid_t b_pid;
		if ((b_pid = become_daemon(TM_NO_CHDIR | TM_NO_UMASK0 | TM_CLOSE_FILES)) == -1)
		{
			throw std::runtime_error("Could not become daemon.");
		}
		if (b_pid > 0)
		{
			return;
		}
		deamonized = true;
	}

	(void)server.listen();
	setup_signals();

	Logger::print("Daemon started with pid: " + std::to_string(getpid()));

	do
	{
		if (server.cycle())
		{
			break;
		}
		if (master.cycle())
		{
			break;
		}
	} while (Taskmaster::running);

	Logger::print(TM_PROJECTD " stopping, this can take a while...", B_GREEN);
	(void)master.stop();
	(void)server.shutdown();

	while (!master.allStopped())
	{
		(void)master.cycle();
	}

	(void)server.stop();

	ignore_signals();
	g_master = nullptr;

	remove_pid_file(pidfile.c_str());

	Logger::print(TM_PROJECTD " stopped", B_GREEN);
}

int
main(int argc, char* const* argv)
{
	(void)argc;

	Logger::printHeader();

	int ch = 0;
	std::string config_file;

	struct tm_getopt_list_s optlist[] = {
		{"configuration", 'c', TM_OPTPARSE_REQUIRED},
		{"nodaemon", 'n', TM_OPTPARSE_NONE},
		{"silent", 's', TM_OPTPARSE_NONE},
		{"help", 'h', TM_OPTPARSE_NONE},
		{"debug", 'd', TM_OPTPARSE_NONE},
		{"version", 'v', TM_OPTPARSE_NONE},
		{nullptr, 0, TM_OPTPARSE_NONE}
	};
	struct tm_getopt_s options;

	tm_getopt_init(&options, argv);
	while ((ch = tm_getopt(&options, optlist, NULL)) != -1)
	{
		switch (ch)
		{
			case 'c':
				if (options.optarg == nullptr)
				{
					Logger::error("No configuration file specified");
					return (TM_FAILURE);
				}
				config_file = options.optarg;
				break;
			case 'n':
				deamonized = true;
				break;
			case 's':
				Logger::silent(true);
				break;
			case 'v':
				std::cout << TM_PROJECTD " " TM_VERSION " - " TM_AUTHOR << std::endl;
				exit(0);
			case 'd':
				Logger::setDebug(true);
				break;
			case 'h':
			default:
				usage(argv[0]);
		}
	}

	Logger::enableFileLogging();
	Logger::init("Starting daemon");

	ignore_signals();
	setup_signal(SIGPIPE, SIG_IGN);

	try
	{
		do
		{
			start_main_loop(config_file);
		} while (Taskmaster::reload);
	}
	catch (const std::exception& e)
	{
		Logger::error(e.what());
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}
