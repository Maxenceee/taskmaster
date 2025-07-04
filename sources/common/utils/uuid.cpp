/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   uuid.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/20 17:12:53 by mgama             #+#    #+#             */
/*   Updated: 2025/06/14 10:51:31 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/utils.hpp"
#include <random>
#include <iomanip>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

static std::uniform_int_distribution<uint16_t> dis3(0, std::numeric_limits<uint16_t>::max());

std::string
uuid_v4()
{
	std::stringstream ss;
	int i;
	ss << std::hex;

	// Première partie : 8 caractères hexadécimaux
	for (i = 0; i < 8; i++) {
		ss << dis(gen);
	}

	ss << "-";

	// Deuxième partie : 4 caractères hexadécimaux
	for (i = 0; i < 4; i++) {
		ss << dis(gen);
	}

	ss << "-4"; // Indique que c'est un UUID version 4
	for (i = 0; i < 3; i++) {
		ss << dis(gen);
	}

	ss << "-";

	// Quatrième partie : un nombre hexadécimal entre 8 et 11
	ss << dis2(gen);
	for (i = 0; i < 3; i++) {
		ss << dis(gen);
	}

	ss << "-";

	// Cinquième partie : 12 caractères hexadécimaux
	for (i = 0; i < 12; i++) {
		ss << dis(gen);
	}

	return ss.str();
}

uint16_t
u_uint16()
{
	static std::set<uint16_t> generated;

	uint16_t value;
	const size_t max_attempts = 10000;

	for (size_t attempt = 0; attempt < max_attempts; ++attempt)
	{
		value = dis3(gen);
		if (generated.insert(value).second)
			return value;
	}

	throw std::runtime_error("No more unique uint16_t values available.");
}
