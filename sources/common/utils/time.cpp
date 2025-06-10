/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:25:34 by mgama             #+#    #+#             */
/*   Updated: 2025/06/10 14:09:05 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

int64_t	getTimestamp()
{
	struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

std::string
format_duration(time_duration duration)
{
	auto secs = duration_cast<std::chrono::seconds>(duration).count();
	int days = static_cast<int>(secs / 86400);
	int hours = static_cast<int>((secs % 86400) / 3600);
	int minutes = static_cast<int>((secs % 3600) / 60);
	int seconds = static_cast<int>(secs % 60);

	std::ostringstream oss;
	if (days > 0) {
		oss << days << "d ";
	}
	oss << std::setfill('0') << std::setw(2) << hours << ":"
		<< std::setw(2) << minutes << ":"
		<< std::setw(2) << seconds;

	return oss.str();
}
