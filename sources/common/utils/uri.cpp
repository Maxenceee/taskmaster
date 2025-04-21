/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   uri.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 13:35:36 by mgama             #+#    #+#             */
/*   Updated: 2025/04/21 13:44:21 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"

std::string
resolve_path(const std::string& url, const std::string& scheme)
{
	std::string path;

	// Vérifier si l'URL commence par `scheme`
	if (url.rfind(scheme, 0) == 0) {
		path = url.substr(scheme.length()); // Supprimer `scheme`
	} else {
		path = url; // Déjà un chemin
	}

	// Vérifier si le chemin est absolu
	if (!path.empty() && path[0] == '/') {
		return path; // Déjà un chemin absolu
	}

	char resolved_path[PATH_MAX];

	// Convertir en chemin absolu si c'est un chemin relatif
	if (realpath(path.c_str(), resolved_path)) {
		return std::string(resolved_path);
	}

	// Si realpath échoue, on tente de préfixer avec le répertoire courant
	if (path.rfind("./", 0) == 0) {
		path = path.substr(2); // Supprimer `./` du début
	}
	if (getcwd(resolved_path, sizeof(resolved_path))) {
		return std::string(resolved_path) + "/" + path;
	}

	// Impossible de résoudre, retourner le chemin original
	return path;
}
