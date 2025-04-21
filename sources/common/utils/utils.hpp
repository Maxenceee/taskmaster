/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 11:38:42 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 17:25:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "tm.hpp"

std::vector<std::string>	split(const std::string &str, char c);
std::vector<std::string>	parseQuotedAndSplit(const std::string &input);
std::vector<std::string>	tokenize(const char *line);
std::vector<std::string>	tokenize(const std::string &input);
std::string					join(const std::vector<std::string> &list, const std::string &c);
std::string					&pop(std::string &str);
std::string					&shift(std::string &str);
std::string					&trim(std::string &str, char c = ' ');
std::string					&to_upper(std::string &str);
std::string					to_upper(const std::string &str);
std::string					&to_lower(std::string &str);
std::string					&capitalize(std::string &str);
std::string					&replace(std::string &buffer, const std::string &searchValue, const std::string &replaceValue);
std::string					&replaceAll(std::string &buffer, char searchValue, char replaceValue);

bool						is_digits(const std::string &str);
bool						is_spaces(const std::string &str);

struct cropoutput {
	const std::string &value;
	cropoutput(const std::string &val) : value(val) {}
};
std::ostream&				operator<<(std::ostream& os, struct cropoutput value);
std::string					cropoutputs(const std::string &input);

std::string					getIPAddress(int addr);
uint32_t					setIPAddress(const std::string &addr);
bool						isIPAddressFormat(const std::string &addr);
bool						isIPAddress(const std::string &addr);

size_t						parseSize(const std::string &size);
std::string					getSize(int size);

time_t						parseTime(const std::string &timeStr);
std::string					getTime(time_t time);

std::string					getSignalName(int signal);

/* uri */

std::string	resolve_path(const std::string& url, const std::string& scheme);

/* file */
int	reopenstds(void);
int tempfile(const char channel[3]);

/* uuid */

std::string	uuid_v4();
uint16_t	u_uint16();

/* time */

int64_t	getTimestamp();

#endif /* UTILS_HPP */