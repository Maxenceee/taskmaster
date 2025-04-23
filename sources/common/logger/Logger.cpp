/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2025/04/23 22:24:59 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool Logger::_debug = false;
pthread_mutex_t Logger::_loggerMutex;
bool Logger::_initiated = false;

Logger::LoggerFileStream Logger::cout;
std::ofstream Logger::_logFile;
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

inline std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayColor& dc)
{
	if (isTTY(os))
		os << dc.color;
	return (os);
}

inline std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayReset&)
{
	if (isTTY(os))
		os << RESET;
	return (os);
}

inline std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayDate&)
{
	auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

	std::tm tm = *std::localtime(&time);

	os << Logger::Color(CYAN) << "[" << std::put_time(&tm, "%H:%M:%S") << "] " << Logger::DisplayReset;
	return (os);
}

inline std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayDay&)
{
	auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

	std::tm tm = *std::localtime(&time);

	os << Logger::Color(CYAN) << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] " << Logger::DisplayReset;
	return (os);
}

inline Logger::LoggerFileStream&
operator<<(Logger::LoggerFileStream& lfs, std::ostream& (*manip)(std::ostream&))
{
	manip(static_cast<std::ostream&>(lfs));
	lfs.flush();
	return lfs;
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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << action << ": New logger session" << std::endl;

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

inline void
Logger::destroy(void)
{
	if (!Logger::_initiated)
		return ;

#ifndef TM_DISABLE_SYSLOG
	closelog();
#endif

	(void)pthread_mutex_destroy(&Logger::_loggerMutex);
}

inline bool
Logger::aquireMutex(void)
{
	if (!Logger::_initiated)
	{
		Logger::init(TM_PROJECT);
	}
	return (pthread_mutex_lock(&Logger::_loggerMutex) == 0);
}

inline bool
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
Logger::syn(const char *msg)
{
	(void)Logger::aquireMutex();

	std::cout << Logger::Color(msg) << std::flush;

	(void)Logger::releaseMutex();
}

void
Logger::syn(const std::string &msg)
{
	Logger::syn(msg.c_str());
}

void
Logger::print(const char *msg, const char *color)
{
	(void)Logger::aquireMutex();

	std::cout << Logger::DisplayDate << Logger::Color(color) << msg << Logger::DisplayReset << std::endl;
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << msg << std::endl;

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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << "Info: " << msg << std::endl;

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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << "Warning: " << msg << std::endl;

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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << "Error: " << msg << std::endl;

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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << "Error: " << msg << ": " << strerror(errno) << std::endl;

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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << "Error: " << msg << ": " << hstrerror(h_errno) << std::endl;

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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << getpid() << "] - " << "Debug: " << msg << std::endl;

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
Logger::enableFileLogging(const std::string& fname)
{
	Logger::_file_logging = true;
	if (Logger::_logFile.is_open())
	{
		Logger::_logFile.close();
	}
	Logger::_logFile.open(fname, std::ios::out | std::ios::app);
}

void
Logger::enableFileLogging(void)
{
	Logger::enableFileLogging(TM_MAIN_LOG_DIR TM_PROJECTD ".log");
}

void
Logger::enableRotationLogging(void)
{
	Logger::_rotation_logging = true;
}
