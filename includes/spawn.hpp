/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spawn.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:58:56 by mgama             #+#    #+#             */
/*   Updated: 2025/05/20 19:52:55 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SPAWN_HPP
#define SPAWN_HPP

// spawn
int	spawn_child(char* const* argv, char* const* envp, int stdin_fd, int stdout_fd, int stderr_fd
#ifdef TM_SPAWN_CHILD_USE_FORK
	, uid_t uid
#endif /* TM_SPAWN_CHILD_USE_FORK */
	, gid_t pgid = 0, const char* dir = nullptr, mode_t mode = 0);

#endif /* SPAWN_HPP */