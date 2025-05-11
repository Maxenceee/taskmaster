/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spawn.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:58:56 by mgama             #+#    #+#             */
/*   Updated: 2025/05/11 16:47:33 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SPAWN_HPP
#define SPAWN_HPP

// spawn
int	spawn_child(char* const* argv, char* const* envp, int stdin_fd, int stdout_fd, int stderr_fd
#ifdef TM_SPAWN_CHILD_USE_FORK
	, uid_t uid
#endif /* TM_SPAWN_CHILD_USE_FORK */
#ifdef TM_SPAWN_CHILD_SUPPORT_PGID
	, int pgid
#endif /* TM_SPAWN_CHILD_SUPPORT_PGID */
	, const char* dir);

#endif /* SPAWN_HPP */