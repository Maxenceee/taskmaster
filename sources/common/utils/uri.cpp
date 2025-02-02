/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   uri.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/02 13:35:36 by mgama             #+#    #+#             */
/*   Updated: 2025/02/02 13:36:47 by mgama            ###   ########.fr       */
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

    // Convertir en chemin absolu si c'est un chemin relatif
    char resolved_path[PATH_MAX];
    if (realpath(path.c_str(), resolved_path)) {
        return std::string(resolved_path);
    }

    // Si realpath échoue, on tente de préfixer avec le répertoire courant
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        return std::string(cwd) + "/" + path;
    }

    // Impossible de résoudre, retourner le chemin original
    return path;
}
