/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:25:34 by mgama             #+#    #+#             */
/*   Updated: 2025/01/18 19:30:04 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

int64_t	getTimestamp()
{
	struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}