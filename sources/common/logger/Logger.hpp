/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:53 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 13:01:56 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "tm.hpp"
#include "pcolors.hpp"

#define TM_MAIN_LOG_FNAME TM_MAIN_LOG_DIR TM_PROJECTD ".log"

enum tm_log_file_channel {
	TM_LOG_FILE_STDOUT	= 0x01,
	TM_LOG_FILE_STDERR	= 0x10,
};

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
	private:
		std::string		_fname;
		std::ofstream	_logFile;
		size_t			_maxSize;

	protected:
		int	overflow(int c) override;
		int	sync(void) override;

		void	openLogFile(void);
		void	checkRotation(void);
		void	renameLogFile(void);

	public:
		LoggerFileStream() :
			std::ostream(this),
			_fname(TM_MAIN_LOG_FNAME),
			_maxSize(TM_MAX_LOG_FILE_SIZE) {};

		void	setFileName(const std::string& fname);
		void	setMaxSize(size_t size);

		void	reopen(void);

		std::ifstream	dump(void) const;
	};

private:
	static bool				_debug;
	static pthread_mutex_t	_loggerMutex;
	static bool				_initiated;

	static pid_t			_pid;

	static LoggerFileStream	cout;
	static LoggerFileStream	cerr;
	static bool				_file_logging;
	static bool				_rotation_logging;

	static bool	aquireMutex(void);
	static bool	releaseMutex(void);

	static void	destroy(void);

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
	static void	enableFileLogging(const std::string& out, const std::string& err);
	static void setLogFileMaxSize(size_t size, int channel);
	static void reopenFileLogging(void);

	static std::ifstream dump(tm_log_file_channel channel);

	friend std::ostream& operator<<(std::ostream& os, const LoggerDisplayColor&);
	friend std::ostream& operator<<(std::ostream& os, const LoggerDisplayDate&);
	friend std::ostream& operator<<(std::ostream& os, const LoggerDisplayDay&);
	friend std::ostream& operator<<(std::ostream& os, const LoggerDisplayReset&);
	friend LoggerFileStream& operator<<(LoggerFileStream& lfs, std::ostream& (*manip)(std::ostream&));
};

#endif /* LOGGER_HPP */