/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   spawn.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 18:58:56 by mgama             #+#    #+#             */
/*   Updated: 2025/03/16 18:35:12 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SPAWN_HPP
#define SPAWN_HPP

// spawn
int	spawn_child(char* const* argv, char* const* envp, int stdin_fd, int stdout_fd, int stderr_fd, int pgid);

#endif /* SPAWN_HPP */