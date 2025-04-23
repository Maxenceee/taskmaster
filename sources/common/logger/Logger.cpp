/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2025/04/23 16:10:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool Logger::_debug = false;
bool Logger::_initiated = false;
pthread_mutex_t Logger::_loggerMutex;
int Logger::_log_fd = -1;
bool Logger::_file_logging = false;
bool Logger::_rotation_logging = false;

static inline bool
isTTY(std::ostream& os)
{
	if (&os == &std::cout)
		return isatty(fileno(stdout));
	else if (&os == &std::cerr)
		return isatty(fileno(stderr));
	return false;
}

std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayColor& dc)
{
	if (isTTY(os))
		os << dc.color;
	return (os);
}

std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayReset&)
{
	if (isTTY(os))
		os << RESET;
	return (os);
}

std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayDate&)
{
	struct tm *tm;
	time_t rawtime;
	char buf[32];

	(void)time(&rawtime);
	tm = localtime(&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	os << Logger::Color(CYAN) << "[" << buf << "] " << Logger::DisplayReset;
	return (os);
}

void
Logger::init(const char *action)
{
	if (Logger::_initiated)
		return ;

#ifndef TM_DISABLE_SYSLOG
	openlog(TM_PROJECT, LOG_PID, LOG_DAEMON);
#endif

	std::cout << Logger::DisplayDate << TM_PREFIX << action << ": New logger session" << std::endl;
	std::cerr << Logger::DisplayDate << TM_PREFIX << action << ": New logger session" << std::endl;

#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_INFO, "%s: New logger session", action);
#endif

	/**
	 * Initialisation du mutex pour éviter les conflits d'affichage
	 */
	(void)pthread_mutex_init(&Logger::_loggerMutex, NULL);
	(void)std::atexit(Logger::destroy);
	Logger::_initiated = true;
}

void
Logger::destroy(void)
{
	if (!Logger::_initiated)
		return ;

#ifndef TM_DISABLE_SYSLOG
	closelog();
#endif

	(void)pthread_mutex_destroy(&Logger::_loggerMutex);
}

bool
Logger::aquireMutex(void)
{
	if (!Logger::_initiated)
	{
		Logger::init(TM_PROJECT);
	}
	return (pthread_mutex_lock(&Logger::_loggerMutex) == 0);
}

bool
Logger::releaseMutex(void)
{
	return (pthread_mutex_unlock(&Logger::_loggerMutex) == 0);
}

void
Logger::printHeader(bool tty_fallback)
{
	if (isTTY(std::cout))
	{
		std::cout << "\n" << Logger::Color(HEADER) << TM_OCTO << Logger::DisplayReset << "\n" << std::setw(12) << "" << Logger::Color(HACKER) << "Taskmaster" << "\n" << Logger::DisplayReset << std::endl;
	}
	else if (tty_fallback)
	{
		std::ofstream tty("/dev/tty");
		if (tty.is_open())
		{
			tty << "\n" << HEADER << TM_OCTO << RESET << "\n" << std::setw(12) << "" << HACKER << "Taskmaster" << "\n" << RESET << std::endl;
			tty.close();
		}
	}
}

void
Logger::cout(const char *msg)
{
	(void)Logger::aquireMutex();
	std::cout << Logger::Color(msg) << std::flush;
	(void)Logger::releaseMutex();
}

void
Logger::cout(const std::string &msg)
{
	Logger::cout(msg.c_str());
}

void
Logger::print(const char *msg, const char *color)
{
	(void)Logger::aquireMutex();
	std::cout << Logger::DisplayDate << Logger::Color(color) << msg << Logger::DisplayReset << std::endl;
#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_INFO, "%s", msg);
#endif
	(void)Logger::releaseMutex();
}

void
Logger::print(const std::string &msg, const char *color)
{
	Logger::print(msg.c_str(), color);
}

void
Logger::info(const char *msg)
{
	(void)Logger::aquireMutex();
	std::cout << Logger::DisplayDate << Logger::Color(YELLOW) << TM_PREFIX << msg << Logger::DisplayReset << std::endl;
#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_INFO, "%s", msg);
#endif
	(void)Logger::releaseMutex();
}

void
Logger::info(const std::string &msg)
{
	Logger::info(msg.c_str());
}

void
Logger::warning(const char *msg)
{
	(void)Logger::aquireMutex();
	std::cerr << Logger::DisplayDate << Logger::Color(B_ORANGE) << TM_PREFIX << msg << Logger::DisplayReset << std::endl;
#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_WARNING, "%s", msg);
#endif
	(void)Logger::releaseMutex();
}

void
Logger::warning(const std::string &msg)
{
	Logger::warning(msg.c_str());
}

void
Logger::error(const char *msg)
{
	(void)Logger::aquireMutex();
	std::cerr << Logger::DisplayDate << Logger::Color(B_RED) << TM_PREFIX << msg << Logger::DisplayReset << std::endl;
#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_ERR, "%s", msg);
#endif
	(void)Logger::releaseMutex();
}

void
Logger::error(const std::string &msg)
{
	Logger::error(msg.c_str());
}

void
Logger::perror(const char *msg)
{
	(void)Logger::aquireMutex();
	std::cerr << Logger::DisplayDate << Logger::Color(B_RED) << TM_PREFIX << msg << ": " << strerror(errno) << Logger::DisplayReset << std::endl;
#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_ERR, "%s: %s", msg, strerror(errno));
#endif
	(void)Logger::releaseMutex();
}

void
Logger::perror(const std::string &msg)
{
	Logger::perror(msg.c_str());
}

void
Logger::pherror(const char *msg)
{
	(void)Logger::aquireMutex();
	std::cerr << Logger::DisplayDate << Logger::Color(B_RED) << TM_PREFIX << msg << ": " << hstrerror(h_errno) << Logger::DisplayReset << std::endl;
#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_ERR, "%s: %s", msg, hstrerror(h_errno));
#endif
	(void)Logger::releaseMutex();
}

void
Logger::pherror(const std::string &msg)
{
	Logger::pherror(msg.c_str());
}

void
Logger::debug(const char *msg, const char *color)
{
	if (!Logger::_debug)
		return ;

	(void)Logger::aquireMutex();
	std::cout << Logger::DisplayDate << Logger::Color(color) << msg << Logger::DisplayReset << std::endl;
#ifndef TM_DISABLE_SYSLOG
	syslog(LOG_DEBUG, "%s", msg);
#endif
	(void)Logger::releaseMutex();
}

void
Logger::debug(const std::string &msg, const char *color)
{
	Logger::debug(msg.c_str(), color);
}

void
Logger::setDebug(bool debug)
{
	Logger::_debug = debug;
}

bool
Logger::isDebug(void)
{
	return (Logger::_debug);
}

void
Logger::enableFileLogging(void)
{
	Logger::_file_logging = true;
}