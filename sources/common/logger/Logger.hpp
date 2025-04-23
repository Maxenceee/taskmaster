/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:53 by mgama             #+#    #+#             */
/*   Updated: 2025/04/23 22:18:44 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "tm.hpp"
#include "pcolors.hpp"

class Logger
{
protected:
	struct LoggerDisplayColor {
		const char *color;
		LoggerDisplayColor(const char *c) : color(c) {}
	};

	struct LoggerDisplayReset {};
	
	struct LoggerDisplayDate {};

	struct LoggerDisplayDay {};

	static const LoggerDisplayColor Color(const char *color) { return LoggerDisplayColor(color); }
	static constexpr LoggerDisplayReset DisplayReset = {};
	static constexpr LoggerDisplayDate DisplayDate = {};
	static constexpr LoggerDisplayDay DisplayDay = {};

	class LoggerFileStream : private std::streambuf, public std::ostream
	{
	public:
		LoggerFileStream() : std::ostream(this) {};

	private:
		int overflow(int c) override
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

		int sync() override
		{
			if (Logger::_file_logging && Logger::_logFile.is_open()) {
				Logger::_logFile.flush();
			}
			return (0);
		}
	};

private:
	static bool				_debug;
	static pthread_mutex_t	_loggerMutex;
	static bool				_initiated;

	static Logger::LoggerFileStream cout;
	static std::ofstream	_logFile;
	static bool				_file_logging;
	static bool				_rotation_logging;

	static bool				aquireMutex(void);
	static bool				releaseMutex(void);

	static void				destroy(void);

	static void				addToFile(const std::stringstream& stream);

public:
	static void init(const char *action);

	static void	printHeader(bool tty_fallback = true);

	static void syn(const char *msg);
	static void syn(const std::string &msg);

	static void	print(const char *msg, const char *color = RESET);
	static void	print(const std::string &msg, const char *color = RESET);

	static void	info(const char *msg);
	static void	info(const std::string &msg);

	static void	warning(const char *msg);
	static void	warning(const std::string &msg);

	static void	error(const char *msg);
	static void	error(const std::string &msg);

	static void	perror(const char *msg);
	static void	perror(const std::string &msg);

	static void	pherror(const char *msg);
	static void	pherror(const std::string &msg);

	static void	debug(const char *msg, const char *color = RESET);
	static void	debug(const std::string &msg, const char *color = RESET);

	static void setDebug(bool debug);
	static bool isDebug(void);

	static void	enableFileLogging(void);
	static void	enableFileLogging(const std::string& fname);
	static void enableRotationLogging(void);

	friend std::ostream& operator<<(std::ostream& os, const Logger::LoggerDisplayColor&);
	friend std::ostream& operator<<(std::ostream& os, const Logger::LoggerDisplayDate&);
	friend std::ostream& operator<<(std::ostream& os, const Logger::LoggerDisplayDay&);
	friend std::ostream& operator<<(std::ostream& os, const Logger::LoggerDisplayReset&);
	friend Logger::LoggerFileStream& operator<<(Logger::LoggerFileStream& lfs, std::ostream& (*manip)(std::ostream&));
};

#endif /* LOGGER_HPP */