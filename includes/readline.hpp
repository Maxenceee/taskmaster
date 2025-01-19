/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readline.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 20:08:18 by mgama             #+#    #+#             */
/*   Updated: 2025/01/19 20:08:58 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READLINE_HPP
#define READLINE_HPP

#include "tm.hpp"
#include <termios.h>

std::string	readline(const std::string& prompt);

#endif /* READLINE_HPP */