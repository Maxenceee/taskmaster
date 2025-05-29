/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 17:45:52 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool			Logger::_debug = false;
pthread_mutex_t Logger::_loggerMutex;
bool			Logger::_initiated = false;
bool			Logger::_silent = false;
pid_t			Logger::_pid = getpid();

Logger::LoggerFileStream	Logger::cout;
Logger::LoggerFileStream	Logger::cerr;
bool						Logger::_file_logging = false;
bool						Logger::_rotation_logging = false;

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

	os << Logger::Color(CYAN) << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "] " << Logger::DisplayReset;
	return (os);
}

inline std::ostream&
operator<<(std::ostream& os, const Logger::LoggerDisplayDay&)
{
	auto now = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(now);

	os << Logger::Color(CYAN) << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " << Logger::DisplayReset;
	return (os);
}

inline Logger::LoggerFileStream&
operator<<(Logger::LoggerFileStream& lfs, std::ostream& (*manip)(std::ostream&))
{
	manip(static_cast<std::ostream&>(lfs));
	return (lfs);
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
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << action << ": New logger session" << std::endl;

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
Logger::enableFileLogging(const std::string& out, const std::string& err)
{
	Logger::_file_logging = true;
	Logger::cout.setFileName(out);
	Logger::cerr.setFileName(err);
}

void
Logger::enableFileLogging(void)
{
	Logger::enableFileLogging(TM_MAIN_LOG_FNAME, TM_MAIN_LOG_FNAME);
}

void
Logger::setLogFileMaxSize(size_t size, int channel)
{
	if (channel & TM_LOG_FILE_STDOUT)
	{
		Logger::cout.setMaxSize(size);
	}
	if (channel & TM_LOG_FILE_STDERR)
	{
		Logger::cerr.setMaxSize(size);
	}
}

void
Logger::reopenFileLogging(void)
{
	if (Logger::_file_logging)
	{
		Logger::cout.reopen();
		Logger::cerr.reopen();
	}
}

void
Logger::silent(bool mode)
{
	Logger::_silent = mode;
}

std::ifstream
Logger::dump(tm_log_file_channel channel)
{
	if (channel & TM_LOG_FILE_STDOUT)
	{
		return (Logger::cout.dump());
	}
	else if (channel & TM_LOG_FILE_STDERR)
	{
		return (Logger::cerr.dump());
	}
	throw std::runtime_error("Invalid log file channel");
}

void
Logger::LoggerFileStream::openLogFile(void)
{
	if (Logger::_file_logging)
	{
		this->_logFile.open(this->_fname, std::ios::out | std::ios::app);
	}
}

inline int
Logger::LoggerFileStream::overflow(int c)
{
	if (c != EOF)
	{
		if (Logger::_file_logging)
		{
			this->_logFile << static_cast<char>(c);
		}
	}
	return (c);
}

inline int
Logger::LoggerFileStream::sync(void)
{
	if (Logger::_file_logging)
	{
		Logger::LoggerFileStream::checkRotation();

		(void)this->_logFile.flush();
	}
	return (0);
}

inline void
Logger::LoggerFileStream::checkRotation(void)
{
	if (Logger::_file_logging && this->_logFile.is_open())
	{
		if (this->_maxSize > 0 && this->_logFile.tellp() > this->_maxSize)
		{
			this->_logFile.close();

			Logger::LoggerFileStream::renameLogFile();

			Logger::LoggerFileStream::openLogFile();
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

	newFileName << this->_fname << "." << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
	std::string oldFileName = this->_fname;

	if (!oldFileName.empty())
	{
		(void)std::rename(oldFileName.c_str(), newFileName.str().c_str());
	}
}

inline void
Logger::LoggerFileStream::reopen(void)
{
	if (this->_logFile.is_open())
	{
		this->_logFile.close();
	}
	Logger::LoggerFileStream::renameLogFile();
	Logger::LoggerFileStream::openLogFile();
}

inline void
Logger::LoggerFileStream::setFileName(const std::string& fname)
{
	this->_fname = fname;
	if (this->_logFile.is_open())
	{
		this->_logFile.close();
	}
	Logger::LoggerFileStream::openLogFile();
}

inline void
Logger::LoggerFileStream::setMaxSize(size_t size)
{
	this->_maxSize = size;
}

inline std::ifstream
Logger::LoggerFileStream::dump(void) const
{
	if (this->_logFile.is_open())
	{
		return (std::ifstream(this->_fname));
	}
	throw std::runtime_error("You should never try to read a log file that is not open");
}
