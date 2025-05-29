/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pid.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 13:35:02 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 18:43:03 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PID_HPP
#define PID_HPP

int		create_pid_file(const char* pid_file);
void	remove_pid_file(const char* pid_file);

#endif /* PID_HPP */