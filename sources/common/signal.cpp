/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/19 15:58:37 by mgama             #+#    #+#             */
/*   Updated: 2025/02/01 15:53:53 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"

void
setup_signal(int sig, void (*handler)(int))
{
	struct sigaction	sa;

	sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
	sa.sa_handler = handler;
	(void)sigaction(sig, &sa, NULL);
}

void
setup_info_signal(int sig, void (*handler)(int, siginfo_t*, void*))
{
	struct sigaction sa;
	sa.sa_sigaction = advanced_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGCHLD, &sa, NULL);
}
