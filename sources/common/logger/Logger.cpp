/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2025/04/23 23:13:24 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool Logger::_debug = false;
pthread_mutex_t Logger::_loggerMutex;
bool Logger::_initiated = false;

Logger::LoggerFileStream Logger::cout;
std::ofstream Logger::_logFile;
std::string Logger::_logFileName;
size_t Logger::_logFileMaxSize = TM_MAX_LOG_FILE_SIZE;
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
Logger::enableFileLogging(const std::string& fname)
{
	Logger::_file_logging = true;
	Logger::_logFileName = fname;
	Logger::openLogFile();
}

void
Logger::enableFileLogging(void)
{
	Logger::enableFileLogging(TM_MAIN_LOG_DIR TM_PROJECTD ".log");
}

void
Logger::setLogFileMaxSize(size_t size)
{
	Logger::_logFileMaxSize = size;
}

void
Logger::openLogFile(void)
{
	if (Logger::_file_logging)
	{
		std::cout << "open file " << Logger::_logFileName << std::endl;
		Logger::_logFile.open(Logger::_logFileName, std::ios::out | std::ios::app);
	}
}

inline int
Logger::LoggerFileStream::overflow(int c)
{
	if (c != EOF)
	{
		if (Logger::_file_logging && Logger::_logFile.is_open())
		{
			Logger::_logFile << static_cast<char>(c);
		}
	}
	return (c);
}

inline int
Logger::LoggerFileStream::sync(void)
{
	if (Logger::_file_logging && Logger::_logFile.is_open())
	{
		Logger::LoggerFileStream::checkRotation();

		Logger::_logFile.flush();
	}
	return (0);
}

inline void
Logger::LoggerFileStream::checkRotation(void)
{
	if (Logger::_file_logging && Logger::_logFile.is_open())
	{
		if (Logger::_logFileMaxSize > 0 && Logger::_logFile.tellp() > Logger::_logFileMaxSize)
		{
			Logger::_logFile.close();

			Logger::LoggerFileStream::renameLogFile();

			Logger::openLogFile();
		}
	}
}

inline void
Logger::LoggerFileStream::renameLogFile(void)
{
	std::ostringstream newFileName;
	auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

	std::tm tm = *std::localtime(&time);

	newFileName << Logger::_logFileName << "." << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
	std::string oldFileName = Logger::_logFileName;

	if (!oldFileName.empty())
	{
		std::rename(oldFileName.c_str(), newFileName.str().c_str());
	}
}
