/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strings.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 13:18:00 by mgama             #+#    #+#             */
/*   Updated: 2025/06/14 10:51:14 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::string		&pop(std::string &str)
{
	if (str.size())
		str.resize(str.size() - 1);
	return (str);
}

std::string		&shift(std::string &str)
{
	if (!str.empty())
		str.erase(str.begin());
	return str;
}

std::vector<std::string>	split(const std::string &str, char c)
{
	std::vector<std::string>	tokens;
	std::string					token;
	std::istringstream			tokenStream(str);

	while (std::getline(tokenStream, token, c))
		tokens.push_back(token);
	return (tokens);
}

std::vector<std::string>	parseQuotedAndSplit(const std::string &input)
{
	std::vector<std::string> result;
	std::string current;
	bool inQuotes = false;

	for (size_t i = 0; i < input.size(); ++i) {
		char c = input[i];

		if (c == '"') {
			inQuotes = !inQuotes;
		} else if (c == ' ' && !inQuotes) {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		} else {
			current += c;
		}
	}

	result.push_back(current);

	return result;
}

std::vector<std::string>	tokenize(const char *line)
{
	return tokenize(std::string(line));
}

std::vector<std::string>	tokenize(const std::string &line)
{
	std::vector<std::string> tokens;
	std::string token;
	bool inQuotes = false;

	for (size_t i = 0; i < line.length(); ++i) {
		char c = line[i];

		if (c == '"') {
			inQuotes = !inQuotes; // Toggle quote state
			token += c;
		} else if (c == '#' && !inQuotes) {
            // Ignore the rest of the line after a '#' if not inside quotes
            break;
        } else if ((isspace(c) || c == '\t') && !inQuotes) {
			if (!token.empty()) {
				trim(token, ' ');
				trim(token, '\t');
				trim(token, '\r');
				tokens.push_back(token);
				token.clear();
			}
		} else if ((c == '{' || c == '}' || c == ';') && !inQuotes) {
			if (!token.empty()) {
				trim(token, ' ');
				trim(token, '\t');
				trim(token, '\r');
				tokens.push_back(token);
				token.clear();
			}
			tokens.push_back(std::string(1, c)); // Add the '{', '}', or ';' as a token
		} else {
			token += c;
		}
	}

	if (!token.empty()) {
		tokens.push_back(token);
	}

	if (inQuotes) {
		throw std::runtime_error("Unterminated quote");
	}

	return tokens;
}

struct StringConcatenator {
	std::string separator;

	StringConcatenator(const std::string& sep) : separator(sep) {}

	std::string operator()(const std::string& acc, const std::string& element) const {
		return acc + separator + element;
	}
};

std::string		join(const std::vector<std::string> &list, const std::string &c)
{
	if (list.empty()) {
		return ("");
	}
	auto nextElement = list.cbegin();
	std::advance(nextElement, 1);
	return (std::accumulate(
		nextElement,
		list.cend(),
		list[0],
		StringConcatenator(c)
	));
}

std::string		&trim(std::string &str, char c)
{
	size_t	i;

	if (!str.size())
		return str;
	i = str.size();
	while (i && str[i - 1] == c)
		i--;
	str.resize(i);
	for (i = 0; str[i] == c; i++);
	str = str.substr(i, std::string::npos);
	return (str);
}

std::string		&to_upper(std::string &str)
{
	std::transform(str.begin(), str.end(),str.begin(), ::toupper);
	return (str);
}

std::string		to_upper(const std::string &str)
{
	std::string tmp = str;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	return (tmp);
}

std::string		&to_lower(std::string &str)
{
	std::transform(str.begin(), str.end(),str.begin(), ::tolower);
	return (str);
}

std::string		&capitalize(std::string &str)
{
	size_t	i = 0;

	to_lower(str);
	str[i] = std::toupper(str[i]);
	while((i = str.find_first_of('-', i + 1)) != std::string::npos)
	{
		if (i + 1 < str.size())
		str[i + 1] = std::toupper(str[i + 1]);
	}
	return (str);
}

std::string		getIPAddress(int addr)
{
	// Similaire à inet_ntoa, renvoie une std::string et non un char *.
	std::string	res;

	res += std::to_string((addr & 0xFF000000) >> 24);
	res += ".";
	res += std::to_string((addr & 0xFF0000) >> 16);
	res += ".";
	res += std::to_string((addr & 0xFF00) >> 8);
	res += ".";
	res += std::to_string(addr & 0xFF);
	return (res);
}

uint32_t	setIPAddress(const std::string &addr)
{
	std::vector<std::string>	tokens = split(addr, '.');
	uint32_t					res = 0;
	if (tokens.size() != 4)
		return (TM_SUCCESS);
	res |= (std::atoi(tokens[0].c_str()) << 24);
	res |= (std::atoi(tokens[1].c_str()) << 16);
	res |= (std::atoi(tokens[2].c_str()) << 8);
	res |= std::atoi(tokens[3].c_str());
	return (res);
}

bool	isIPAddressFormat(const std::string &addr)
{
	std::vector<std::string> tokens = split(addr, '.');
	if (tokens.size() != 4)
		return (false);
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (!is_digits(tokens[i]))
			return (false);
	}
	return (true);
}

bool	isIPAddress(const std::string &addr)
{
	std::vector<std::string> tokens = split(addr, '.');
	if (tokens.size() != 4)
		return (false);
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (!is_digits(tokens[i]) || std::atoi(tokens[i].c_str()) < 0 || std::atoi(tokens[i].c_str()) > 255)
			return (false);
	}
	return (true);
}

std::string		&replace(std::string &buffer, const std::string &searchValue, const std::string &replaceValue)
{
	std::string	sv(searchValue);
	std::string	rv(replaceValue);
	std::size_t found_place = buffer.find(sv);

	while (found_place < std::string::npos)
	{
		buffer.erase(found_place, sv.length());
		buffer.insert(found_place, replaceValue);
		found_place = buffer.find(sv, found_place + 1);
	}
	return (buffer);
}

std::string &replaceAll(std::string &buffer, char searchValue, char replaceValue)
{
	size_t pos = 0;
	while ((pos = buffer.find(searchValue, pos)) != std::string::npos) {
		size_t end_pos = pos;
		// Find the end of the successive occurrences
		while (end_pos < buffer.size() && buffer[end_pos] == searchValue) {
			++end_pos;
		}
		// Replace all occurrences with a single one
		buffer.replace(pos, end_pos - pos, 1, replaceValue);
		pos += 1; // Move past the replaced character
	}
	return buffer;
}

size_t	parseSize(const std::string &size)
{
	size_t	ret = 0;
	size_t	i = 0;

	while (i < size.size() && size[i] >= '0' && size[i] <= '9')
	{
		ret = ret * 10 + size[i] - '0';
		i++;
	}
	if (i < size.size())
	{
		if (size[i] == 'b' || size[i] == 'B')
			ret *= 1;
		else if (size[i] == 'k' || size[i] == 'K')
			ret *= 1024;
		else if (size[i] == 'm' || size[i] == 'M')
			ret *= 1024 * 1024;
		else if (size[i] == 'g' || size[i] == 'G')
			ret *= 1024 * 1024 * 1024;
		else
			return (-1);
		if (i + 1 < size.size())
			return (-1);
	}
	return (ret);
}

std::string	getSize(int size)
{
	std::string	ret;
	if (size < 1024)
		ret = std::to_string(size) + "B";
	else if (size < 1024 * 1024)
		ret = std::to_string(size / 1024) + "KB";
	else if (size < 1024 * 1024 * 1024)
		ret = std::to_string(size / (1024 * 1024)) + "MB";
	else
		ret = std::to_string(size / (1024 * 1024 * 1024)) + "GB";
	return (ret);
}

time_t	parseTime(const std::string &timeStr)
{
	time_t	ret = 0;
	size_t	i = 0;

	while (i < timeStr.size() && timeStr[i] >= '0' && timeStr[i] <= '9')
	{
		ret = ret * 10 + timeStr[i] - '0';
		i++;
	}
	if (i < timeStr.size())
	{
		if (timeStr[i] == 's' || timeStr[i] == 'S')
			ret *= 1000;
		else if (timeStr[i] == 'm' || timeStr[i] == 'M')
			ret *= 60 * 1000;
		else if (timeStr[i] == 'h' || timeStr[i] == 'H')
			ret *= 60 * 60 * 1000;
		else if (timeStr[i] == 'd' || timeStr[i] == 'D')
			ret *= 24 * 60 * 60 * 1000;
		else
			return (-1);
		if (i + 1 < timeStr.size())
			return (-1);
	}
	return (ret);
}

std::string	getTime(time_t time)
{
	std::string	ret;
	if (time < 60 * 1000)
		ret = std::to_string(time / 1000) + "s";
	else if (time < 60 * 60 * 1000)
		ret = std::to_string(time / (60 * 1000)) + "m";
	else if (time < 24 * 60 * 60 * 1000)
		ret = std::to_string(time / (60 * 60 * 1000)) + "h";
	else
		ret = std::to_string(time / (24 * 60 * 60 * 1000)) + "d";
	return (ret);
}

std::ostream& operator<<(std::ostream& os, struct cropoutput value) {
	if (value.value.size() > 1000)
		os << value.value.substr(0, 1000) << "... " << getSize(value.value.size() - 1000) << " more";
	else
		os << value.value;
	return os;
}

std::string	cropoutputs(const std::string &input)
{
	if (input.size() > 1000)
		return (input.substr(0, 1000) + "... " + getSize(input.size() - 1000) + " more");
	return (input);
}

bool is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

bool is_spaces(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isspace);
}

std::string	getSignalName(int signal)
{
	if (signal < 0 || signal >= NSIG)
		return ("Unknown signal");

#ifdef __APPLE__
	return ("SIG" + to_upper(sys_signame[signal]));
#else
	return (std::string("SIG") + sigabbrev_np(signal));
#endif /* __APPLE__ */
}
