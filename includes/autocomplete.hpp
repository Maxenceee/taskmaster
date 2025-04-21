/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   autocomplete.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 13:26:18 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 13:29:22 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTOCOMPLETE_HPP
#define AUTOCOMPLETE_HPP

#include "tm.hpp"
#include <readline/readline.h>
#include <readline/history.h>

struct CommandNodeUsage {
	std::string usage;
	std::string description;
};

struct CommandNode {
	std::string								name;							// Nom de la commande
	std::vector<struct CommandNodeUsage> 	usages;							// Liste des variantes d'usage de la commande
	bool									visible;						// Indique si la commande doit être affichée dans l'aide
	bool									ripple_autocomplete;			// Indique si l'autocomplétion doit être propagée aux sous-commandes
	char*									(*suggest)(const char*, int);	// Pointeur vers la fonction de rappel associée à la commande
};

char*	get_process_name(const char* text, int state);

void	show_help();
void	show_command_info(const std::string &cmd);

char**	autocomplete(const char *text, int start, int end);

#endif /* AUTOCOMPLETE_HPP */