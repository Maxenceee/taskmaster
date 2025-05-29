/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LoggerMethods.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/23 22:57:19 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 17:44:08 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

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
Logger::syn(const char *msg)
{
	(void)Logger::aquireMutex();

	if (false == Logger::_silent)
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

	if (false == Logger::_silent)
		std::cout << Logger::DisplayDate << Logger::Color(color) << msg << Logger::DisplayReset << std::endl;
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << msg << std::endl;

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

	if (false == Logger::_silent)
		std::cout << Logger::DisplayDate << Logger::Color(YELLOW) << msg << Logger::DisplayReset << std::endl;
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << "Info: " << msg << std::endl;

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

	if (false == Logger::_silent)
		std::cerr << Logger::DisplayDate << Logger::Color(ORANGE) << msg << Logger::DisplayReset << std::endl;
	Logger::cerr << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << "Warning: " << msg << std::endl;

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

	if (false == Logger::_silent)
		std::cerr << Logger::DisplayDate << Logger::Color(RED) << msg << Logger::DisplayReset << std::endl;
	Logger::cerr << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << "Error: " << msg << std::endl;

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

	if (false == Logger::_silent)
		std::cerr << Logger::DisplayDate << Logger::Color(RED) << msg << ": " << strerror(errno) << Logger::DisplayReset << std::endl;
	Logger::cerr << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << "Error: " << msg << ": " << strerror(errno) << std::endl;

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

	if (false == Logger::_silent)
		std::cerr << Logger::DisplayDate << Logger::Color(RED) << msg << ": " << hstrerror(h_errno) << Logger::DisplayReset << std::endl;
	Logger::cerr << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << "Error: " << msg << ": " << hstrerror(h_errno) << std::endl;

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

	if (false == Logger::_silent)
		std::cout << Logger::DisplayDate << Logger::Color(color) << msg << Logger::DisplayReset << std::endl;
	Logger::cout << Logger::DisplayDay << TM_PROJECTD << "[" << Logger::_pid << "] - " << "Debug: " << msg << std::endl;

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
