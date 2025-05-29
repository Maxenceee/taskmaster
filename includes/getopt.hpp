/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   getopt.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 21:59:51 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 18:02:26 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TM_OPTPARSE_H
#define TM_OPTPARSE_H

struct tm_getopt_s
{
	char**	argv;
	int		permute;
	int		optind;
	int		optopt;
	char*	optarg;
	char	errmsg[64];
	int		subopt;
};

enum tm_getopt_argtype_e {
	TM_OPTPARSE_NONE,
	TM_OPTPARSE_REQUIRED,
	TM_OPTPARSE_OPTIONAL
};

struct tm_getopt_list_s
{
	const char*	longname;
	int 		shortname;
	enum tm_getopt_argtype_e argtype;
};

/**
 * Initializes the parser state.
 */
void tm_getopt_init(struct tm_getopt_s *options, char* const* argv);

int tm_getopt(struct tm_getopt_s *options, const struct tm_getopt_list_s *longopts, int *longindex);

#define TM_OPTPARSE_MSG_INVALID "invalid option"
#define TM_OPTPARSE_MSG_MISSING "option requires an argument"
#define TM_OPTPARSE_MSG_TOOMANY "option takes no arguments"

#endif /* TM_OPTPARSE_H */