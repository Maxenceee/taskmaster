/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:53 by mgama             #+#    #+#             */
/*   Updated: 2025/04/23 20:38:16 by mgama            ###   ########.fr       */
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

	struct LoggerDisplayDate {};

	struct LoggerDisplayReset {};

	static const LoggerDisplayColor Color(const char *color) { return LoggerDisplayColor(color); }
	static constexpr LoggerDisplayReset DisplayReset = {};
	static constexpr LoggerDisplayDate DisplayDate = {};

	class LoggerFileStream : public std::ostream
	{
	private:
		std::ostringstream _buffer;

	public:
		LoggerFileStream();

		~LoggerFileStream();

		void flushToFile()
		{
			if (Logger::_file_logging && Logger::_logFile.is_open())
			{
				Logger::_logFile << _buffer.str();
				Logger::_logFile.flush();
				_buffer.str("");
				_buffer.clear();
			}
		}
	};

private:
	static bool				_debug;
	static pthread_mutex_t	_loggerMutex;
	static bool				_initiated;

	static Logger::LoggerFileStream filestream;
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

	static void cout(const char *msg);
	static void cout(const std::string &msg);

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
	static void enableRotationLogging(void);

	friend std::ostream& operator<<(std::ostream& os, const Logger::LoggerDisplayColor&);
	friend std::ostream& operator<<(std::ostream& os, const Logger::LoggerDisplayDate&);
	friend std::ostream& operator<<(std::ostream& os, const Logger::LoggerDisplayReset&);
	friend Logger::LoggerFileStream& operator<<(Logger::LoggerFileStream& lfs, std::ostream& (*manip)(std::ostream&));
};

#endif /* LOGGER_HPP */