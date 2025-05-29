/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 07:59:30 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 20:21:49 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "inipp.hpp"
#include "logger/Logger.hpp"
#include "utils/utils.hpp"
#include "taskmaster/Taskmaster.hpp"

template<typename T>
inline static std::optional<T>
_get(const std::map<std::string, T>& map, const std::string& key) noexcept
{
	auto it = map.find(key);
	if (it != map.end())
		return (it->second);
	return (std::nullopt);
}

template<typename T, typename U>
inline static T
_or_throw(const std::optional<T>& opt, const U& msg)
{
	if (opt.has_value())
		return (opt.value());
	throw std::invalid_argument(msg);
}

inline static int
_integer(const std::optional<std::string>& str, const int _default)
{
	if (!str.has_value() || str->empty())
		return (_default);

	if (!is_digits(*str))
		throw std::invalid_argument("Invalid integer value: " + *str);

	try
	{
		return (std::stoi(*str));
	}
	catch(...)
	{
		throw std::invalid_argument("Invalid integer value: " + *str);
	}
}

inline static uint16_t
_octal_type(const std::optional<std::string>& str, const uint16_t _default)
{
	if (!str || str->empty())
		return (_default);
	
	if (!is_digits(*str))
		throw std::invalid_argument("Invalid octal value: " + *str);

	try
	{
		return (std::stoi(*str, nullptr, 8));
	}
	catch(...)
	{
		throw std::invalid_argument("Invalid octal value: " + *str);
	}
}

inline static bool
_boolean(const std::optional<std::string>& str, const bool _default)
{
	if (!str || str->empty())
		return (_default);

	if (TRUTHY_STRINGS(*str))
		return (true);
	else if (FALSY_STRINGS(*str))
		return (false);

	throw std::invalid_argument("Invalid boolean value: " + *str);
}

inline static tm_config_auto_restart
_auto_restart(const std::optional<std::string>& str, const tm_config_auto_restart _default)
{
	if (!str || str->empty())
		return (_default);
	
	if (TRUTHY_STRINGS(*str))
		return (TM_CONF_AUTORESTART_TRUE);
	else if (FALSY_STRINGS(*str))
		return (TM_CONF_AUTORESTART_FALSE);
	else if (*str == "unexpected")
		return (TM_CONF_AUTORESTART_UNEXPECTED);

	throw std::invalid_argument("Invalid auto-restart value: " + *str);
}

inline static int
_signal_number(const std::optional<std::string>& str, const int _default)
{
	if (!str || str->empty())
		return (_default);

	if (str == "SIGTERM")
		return (SIGTERM);
	else if (str == "SIGHUP")
		return (SIGHUP);
	else if (str == "SIGINT")
		return (SIGINT);
	else if (str == "SIGQUIT")
		return (SIGQUIT);
	else if (str == "SIGKILL")
		return (SIGKILL);
	else if (str == "SIGUSR1")
		return (SIGUSR1);
	else if (str == "SIGUSR2")
		return (SIGUSR2);

	throw std::invalid_argument("Invalid signal value: " + *str);
}

inline static std::string
_existring_dirpath(const std::optional<std::string>& str, const std::string& _default)
{
	if (!str || str->empty())
		return (_default);

	std::string nv = *str;

	if (nv[0] == '~')
	{
		const char* home = getenv("HOME");
		if (home)
			nv.replace(0, 1, home);
	}

	size_t pos = nv.find_last_of("/\\");
	std::string dir = (pos == std::string::npos) ? "" : nv.substr(0, pos);

	if (dir.empty())
		return (nv);

	struct stat info;
	if (stat(dir.c_str(), &info) == 0 && (info.st_mode & S_IFDIR))
		return (nv);

	throw std::invalid_argument("The directory named as part of the path " + *str + " does not exist");
}

inline static size_t
_max_bytes(const std::optional<std::string>& str, const size_t _default)
{
	if (!str || str->empty())
		return (_default);

	static const std::unordered_map<std::string, size_t> suffixes = {
		{"kb", 1024ULL},
		{"mb", 1024ULL * 1024},
		{"gb", 1024ULL * 1024 * 1024},
	};

	std::string input = *str;
	std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) {
		return std::tolower(c);
	});

	if (input.size() >= 2) {
		std::string suffix = input.substr(input.size() - 2);
		auto it = suffixes.find(suffix);
		if (it != suffixes.end()) {
			std::string numberPart = input.substr(0, input.size() - 2);
			try
			{
				return static_cast<size_t>(std::stoull(numberPart)) * it->second;
			}
			catch (...)
			{
				throw std::invalid_argument("Invalid size value: " + *str);
			}
		}
	}

	try
	{
		return static_cast<size_t>(std::stoull(input)) * _default;
	}
	catch (...)
	{
		throw std::invalid_argument("Invalid size value: " + *str);
	}
}

inline static uid_t
_name_to_uid(const std::optional<std::string>& name)
{
	if (!name || name->empty())
		return (getuid());

	struct passwd* pw = getpwnam(name->c_str());
	if (pw == nullptr)
	{
		throw std::invalid_argument("Invalid user name: " + *name);
	}
	return (pw->pw_uid);
}

inline static gid_t
_name_to_gid(const std::optional<std::string>& name)
{
	if (!name || name->empty())
		return (getgid());

	struct passwd* pw = getpwnam(name->c_str());
	if (pw == nullptr)
	{
		throw std::invalid_argument("Invalid group name: " + *name);
	}
	return (pw->pw_gid);
}

inline static tm_Config::UnixServer::owner
_user_separated_by_colon(const std::optional<std::string>& str)
{
	tm_Config::UnixServer::owner _target;

	if (!str || str->empty())
	{
		_target.uid = getuid();
		_target.gid = getgid();
		return (_target);
	}

	size_t pos = str->find(':');
	if (pos != std::string::npos)
	{
		auto uid = str->substr(0, pos);
		auto git = str->substr(pos + 1);
		if (uid.empty() || git.empty())
		{
			throw std::invalid_argument("Invalid user:group format: " + *str);
		}
		_target.uid = _name_to_uid(uid);
		_target.gid = _name_to_gid(git);
		return (_target);
	}
	else
	{
		_target.uid = _name_to_uid(str);
		_target.gid = _name_to_gid(str);
		return (_target);
	}
}

template <typename T = std::string, typename U = std::vector<T>>
inline static U
_list(const std::optional<std::string>& str, const U& _default = U())
{
	if (!str || str->empty())
		return (_default);

	U result;
	std::istringstream iss(*str);
	std::string item;
	while (std::getline(iss, item, ','))
	{
		result.push_back(static_cast<T>(item));
	}
	return (result);
}

inline static std::vector<int>
_exit_codes(const std::optional<std::string>& str, const std::vector<int>& _default)
{
	if (!str || str->empty())
		return (_default);

	std::vector<int> result;
	std::istringstream iss(*str);
	std::string item;
	while (std::getline(iss, item, ','))
	{
		try
		{
			auto code = std::stoi(item);
			if (code < 0 || code > 255)
				throw std::out_of_range("Exit code out of range: " + item);
			result.push_back(code);
		}
		catch (const std::invalid_argument&)
		{
			throw std::invalid_argument("Invalid exit code: " + item);
		}
	}
	return (result);
}

inline static std::vector<std::string>
_exec(const std::optional<std::string>& str)
{
	if (!str || str->empty())
		throw std::invalid_argument("You must provide a command to execute");
		
	auto l = split(str.value());
	if (l.size() == 0)
		throw std::invalid_argument("You must provide a command to execute");

	auto cmd = l.front();

	if (cmd.find('/') != 0)
	{
		throw std::invalid_argument("The command '" + *str + "' must have an absolute path");
	}

	if (access(cmd.c_str(), F_OK | X_OK) != 0)
	{
		throw std::invalid_argument("The command '" + *str + "' does not exist or is not executable");
	}
	return (l);
}

inline static tm_Config::UnixServer
_parseUnixServerConfig(const std::map<std::string, std::string>& section)
{
	tm_Config::UnixServer config;

	config.file = _get(section, "file").value_or(TM_SOCKET_PATH);
	config.chmod = _octal_type(_get(section, "chmod"), 0660);
	config.chown = _user_separated_by_colon(_get(section, "chown"));

	return (config);
}

inline static tm_Config::Daemon
_parseDaemonConfig(const std::map<std::string, std::string>& section)
{
	tm_Config::Daemon config;

	config.logfile = _existring_dirpath(_get(section, "logfile"), TM_LOG_FILE);
	config.pidfile = _existring_dirpath(_get(section, "pidfile"), TM_PID_FILE);
	config.logfile_maxbytes = _max_bytes(_get(section, "logfile_maxbytes"), 1024 * 1024);
	config.umask = _octal_type(_get(section, "umask"), 022);
	config.nodaemon = _boolean(_get(section, "nodaemon"), false);
	config.childlogdir = _existring_dirpath(_get(section, "childlogdir"), TM_MAIN_LOG_DIR);
	config.user = _name_to_uid(_get(section, "user"));
	config.directory = _existring_dirpath(_get(section, "directory"), TM_MAIN_LOG_DIR);
	config.environment = _list(_get(section, "environment"));

	return (config);
}

inline static tm_Config::Program
_parseProgramConfig(const std::string& section_name, const std::map<std::string, std::string>& section)
{
	tm_Config::Program config;

	size_t colpos = section_name.find(":");
	if (colpos == std::string::npos)
	{
		throw std::invalid_argument("Invalid program name format: " + section_name);
	}

	if (colpos == 0 || colpos == section_name.size() - 1)
	{
		throw std::invalid_argument("Program name cannot be empty: " + section_name);
	}

	config.name = section_name.substr(1 + colpos);
	config.command = _exec(_get(section, "command"));
	config.raw_command = _or_throw(_get(section, "command"), "");
	config.numprocs = _integer(_get(section, "numprocs"), 1);
	config.priority = _integer(_get(section, "priority"), 999);
	config.autostart = _boolean(_get(section, "autostart"), true);
	config.startsecs = _integer(_get(section, "startsecs"), 1);
	config.startretries = _integer(_get(section, "startretries"), 3);
	config.autorestart = _auto_restart(_get(section, "autorestart"), TM_CONF_AUTORESTART_UNEXPECTED);
	config.exitcodes = _exit_codes(_get(section, "exitcodes"), { 0 });
	config.stopsignal = _signal_number(_get(section, "stopsignal"), SIGTERM);
	config.stopwaitsecs = _integer(_get(section, "stopwaitsecs"), 10);
	config.stopasgroup = _boolean(_get(section, "stopasgroup"), false);
	config.killasgroup = _boolean(_get(section, "killasgroup"), false);
	config.user = _name_to_uid(_get(section, "user"));
	config.stdout_logfile = _existring_dirpath(_get(section, "stdout_logfile"), "");
	config.stderr_logfile = _existring_dirpath(_get(section, "stderr_logfile"), "");
	config.environment = _list(_get(section, "environment"));
	config.directory = _existring_dirpath(_get(section, "directory"), TM_CURRENT_DIR);
	config.umask = _octal_type(_get(section, "umask"), 022);

	return (config);
}

inline static std::string
_search_and_load_config(void)
{
	const char* env_config = getenv("TASKMASTER_CONFIG");
	if (env_config && access(env_config, R_OK) == 0)
		return std::string(env_config);

	static const std::vector<const char *> default_paths = {
		"/etc/" TM_PROJECTD ".conf",
		"/etc/taskmaster/" TM_PROJECTD ".conf",
		"/usr/local/etc/taskmaster/" TM_PROJECTD ".conf",
		TM_CURRENT_DIR TM_PROJECTD ".conf"
	};

	for (const auto& path : default_paths)
	{
		if (access(path, R_OK) == 0)
			return (path);
	}

	throw std::runtime_error("No valid configuration file found");
}

tm_Config
_parseConfig(const std::string& filename)
{
	inipp::Ini<char> ini;
	tm_Config new_conf;

	std::ifstream is(filename);
	ini.parse(is);
	for (auto section : ini.sections)
	{
		if (section.first.find("program:") == 0)
		{
			new_conf.programs.push_back(_parseProgramConfig(section.first, section.second));
		}
		else if (section.first != "unix_server" && section.first != "taskmasterd")
		{
			throw std::invalid_argument("Unknown section: " + section.first);
		}
	}
	new_conf.server = _parseUnixServerConfig(_get(ini.sections, "unix_server").value_or(std::map<std::string, std::string>{}));
	new_conf.daemon = _parseDaemonConfig(_get(ini.sections, "taskmasterd").value_or(std::map<std::string, std::string>{}));

	if (Logger::isDebug())
	{
		std::ostringstream oss;
		oss << "Parsed config file:" << "\n";
		oss << new_conf;
		Logger::debug(oss.str());
	}

	return (new_conf);
}

int
Taskmaster::readconfig(void)
{
	try
	{
		if (this->_config_file.empty())
		{
			this->_read_config = _parseConfig(_search_and_load_config());
		}
		else
		{
			this->_read_config = _parseConfig(this->_config_file);
		}
	}
	catch(const std::exception& e)
	{
		Logger::error("Error while parsing config file:");
		Logger::error(e.what());
		return (TM_FAILURE);
	}

	return (TM_SUCCESS);
}

void
Taskmaster::_remove(const ProcessGroup *process)
{
	auto it = std::find(this->_processes.begin(), this->_processes.end(), process);
	if (it != this->_processes.end())
	{
		auto group = *it;
		group->remove();
		this->_transitioning.push_back(group);
		this->_processes.erase(it);
	}
}

inline static void
_diff_process_vs_config(
	const std::vector<ProcessGroup*>&		processes,
	const std::vector<tm_Config::Program>&	programs,
	std::vector<const tm_Config::Program*>&	to_add,
	std::vector<ProcessGroup*>&				to_remove
)
{
	std::unordered_set<std::string> progNames;
	progNames.reserve(programs.size());
	for (const auto& p : programs)
	{
		progNames.insert(p.name);
	}

	std::unordered_set<std::string> procNames;
	procNames.reserve(processes.size());
	for (const auto grp : processes)
	{
		const std::string& name = grp->getName();
		procNames.insert(name);
		if (!progNames.contains(name))
		{
			to_remove.push_back(grp);
		}
	}

	for (const auto& p : programs)
	{
		if (!procNames.contains(p.name))
		{
			to_add.push_back(&p);
		}
	}
}

inline static void
_diff_conf_programs(
    const std::vector<tm_Config::Program>& old_programs,
    const std::vector<tm_Config::Program>& new_programs,
	std::unordered_set<std::string>& old_names,
	std::unordered_set<std::string>& new_names
) {
    for (const auto& prog : old_programs) {
        old_names.insert(prog.name);
    }

    for (const auto& prog : new_programs) {
        new_names.insert(prog.name);
    }
}

std::string
Taskmaster::update(void)
{
	this->_active_config = this->_read_config;

	/** Daemon **/

	(void)umask(this->_active_config.daemon.umask);

	Logger::setLogFileMaxSize(this->_active_config.daemon.logfile_maxbytes, TM_LOG_FILE_STDOUT | TM_LOG_FILE_STDERR);

	uid_t current_uid = getuid();
	if (this->_active_config.daemon.user != current_uid)
	{
		if (current_uid != 0)
		{
			Logger::error("Cannot change user from " + std::to_string(current_uid) + " to " + std::to_string(this->_active_config.daemon.user) + ": not running as root");
			return ("Cannot change user: not running as root\n");
		}
		if (setuid(this->_active_config.daemon.user) == -1)
		{
			Logger::perror("Failed to set user for the daemon");
			return ("Failed to set user for the daemon\n");
		}
	}

	if (this->_active_config.daemon.directory != TM_CURRENT_DIR)
	{
		if (chdir(this->_active_config.daemon.directory.c_str()) == -1)
		{
			Logger::perror("Failed to change working directory to " + this->_active_config.daemon.directory);
			return ("Failed to change working directory\n");
		}
	}

	/** Processes **/

	std::vector<const tm_Config::Program*> to_add;
	std::vector<ProcessGroup*> to_remove;

	std::ostringstream oss;

	_diff_process_vs_config(this->_processes, this->_active_config.programs, to_add, to_remove);

	for (const auto* process : to_remove)
	{
		oss << process->getName() << ": removed process group" << "\n";
		this->_remove(process);
	}
	to_remove.clear();
	for (const auto process : this->_processes)
	{
		auto conf = std::find_if(
			this->_active_config.programs.begin(),
			this->_active_config.programs.end(),
			[&](const tm_Config::Program& program) {
				return program.name == process->getName();
			}
		);
		// This should never happen, but just in case
		if (conf == this->_active_config.programs.end())
		{
			Logger::error("Process not bound to any config: " + process->getName());
			(void)process->remove();
			to_remove.push_back(process);
			continue;
		}
		process->update(*conf);
	}
	for (const auto* process : to_remove)
	{
		oss << process->getName() << ": removed process group" << "\n";
		this->_remove(process);
	}
	for (const auto* program : to_add)
	{
		auto newp = new ProcessGroup(*program);
		this->_processes.push_back(newp);
		oss << program->name << ": added process group" << "\n";
	}

	return (oss.str());
}

std::string
Taskmaster::getConfChanges(void) const
{
	std::unordered_set<std::string> old_names;
	std::unordered_set<std::string> new_names;
	std::ostringstream oss;

	_diff_conf_programs(this->_active_config.programs, this->_read_config.programs, old_names, new_names);

	if (new_names.size() == old_names.size())
	{
		return ("No config updates to processes\n");
	}

    for (const auto& name : old_names) {
        if (new_names.find(name) == new_names.end()) {
            oss << name << ": disappeared\n";
        }
    }

    for (const auto& name : new_names) {
        if (old_names.find(name) == old_names.end()) {
            oss << name << ": available\n";
        }
    }

	return (oss.str());
}
