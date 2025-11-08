/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   envstore.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:12:28 by mgama             #+#    #+#             */
/*   Updated: 2025/11/08 20:13:06 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENVSTORE_HPP
#define ENVSTORE_HPP

#include "tm.hpp"

class EnvStore
{
public:
	struct Entry {
		std::string key;
		std::string value;
	};

private:
	std::vector<Entry> _entries;
	std::unordered_map<std::string, size_t> _index;

	mutable std::vector<std::string> _envStringsCache;
	mutable std::vector<char*> _envCArrayCache;
	mutable bool _cacheValid = false;

	static Entry
	parseEnvLine(const char* line)
	{
		const char* eq = std::strchr(line, '=');
		if (!eq)
			return {line, ""};

		std::string key(line, eq - line);
		std::string value(eq + 1);
		return {std::move(key), std::move(value)};
	}

	void
	invalidateCache() const
	{
		_cacheValid = false;
		_envStringsCache.clear();
		_envCArrayCache.clear();
	}

public:
	EnvStore() = default;

	explicit
	EnvStore(char** envp)
	{
		if (!envp)
			return;

		for (size_t i = 0; envp[i]; ++i)
		{
			Entry e = parseEnvLine(envp[i]);
			if (!e.key.empty())
			{
				_index[e.key] = _entries.size();
				_entries.push_back(std::move(e));
			}
		}
		invalidateCache();
	}

	void
	set(const std::vector<std::string>& envp)
	{
		for (const auto& line : envp)
		{
			Entry e = parseEnvLine(line.c_str());
			if (!e.key.empty())
			{
				set(e.key, e.value);
			}
		}
	}

	void
	set(const std::string& key, const std::string& value)
	{
		auto it = _index.find(key);
		if (it != _index.end())
		{
			if (value.empty())
			{
				size_t pos = it->second;
				size_t last = _entries.size() - 1;

				if (pos != last)
				{
					std::swap(_entries[pos], _entries[last]);
					_index[_entries[pos].key] = pos;
				}

				_entries.pop_back();
				_index.erase(it);
			}
			else
			{
				_entries[it->second].value = value;
			}
		}
		else if (!value.empty())
		{
			_index[key] = _entries.size();
			_entries.push_back({key, value});
		}
		invalidateCache();
	}

	const std::vector<Entry>&
	entries(void) const noexcept
	{
		return _entries;
	}

	size_t
	size() const noexcept
	{
		return _entries.size();
	}

	char* const*
	toEnvpStrings() const
	{
		if (!_cacheValid)
		{
			_envStringsCache.clear();
			_envCArrayCache.clear();

			_envStringsCache.reserve(_entries.size());
			_envCArrayCache.reserve(_entries.size() + 1);

			for (auto& e : _entries)
			{
				_envStringsCache.push_back(e.key + "=" + e.value);
				_envCArrayCache.push_back(const_cast<char*>(_envStringsCache.back().c_str()));
			}

			_envCArrayCache.push_back(nullptr);
			_cacheValid = true;
		}

		return _envCArrayCache.data();
	}
};

#endif /* ENVSTORE_HPP */