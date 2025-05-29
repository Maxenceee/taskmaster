/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   getopt.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/22 17:47:49 by mgama             #+#    #+#             */
/*   Updated: 2025/05/29 17:27:14 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "getopt.hpp"

static int
tm_getopt_error(struct tm_getopt_s *options, const char* msg, const char* data)
{
	unsigned p = 0;
	const char* sep = " -- '";
	while (*msg)
		options->errmsg[p++] = *msg++;
	while (*sep)
		options->errmsg[p++] = *sep++;
	while (p < sizeof(options->errmsg) - 2 && *data)
		options->errmsg[p++] = *data++;
	options->errmsg[p++] = '\'';
	options->errmsg[p++] = '\0';
	return '?';
}	

void
tm_getopt_init(struct tm_getopt_s *options, char* const* argv)
{
	options->argv = (char**)argv;
	options->permute = 1;
	options->optind = argv[0] != 0;
	options->subopt = 0;
	options->optarg = 0;
	options->errmsg[0] = '\0';
}

static int
tm_getopt_is_dashdash(const char* arg)
{
	return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] == '\0';
}

static int
tm_getopt_is_shortopt(const char* arg)
{
	return arg != 0 && arg[0] == '-' && arg[1] != '-' && arg[1] != '\0';
}

static int
tm_getopt_is_longopt(const char* arg)
{
	return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

static void
tm_getopt_permute(struct tm_getopt_s* options, int index)
{
	char* nonoption = options->argv[index];
	for (int i = index; i < options->optind - 1; i++)
		options->argv[i] = options->argv[i + 1];
	options->argv[options->optind - 1] = nonoption;
}

static int
tm_getopt_argtype(const char* optstring, char c)
{
	int count = TM_OPTPARSE_NONE;
	if (c == ':')
		return -1;
	for (; *optstring && c != *optstring; optstring++);
	if (!*optstring)
		return -1;
	if (optstring[1] == ':')
		count += optstring[2] == ':' ? 2 : 1;
	return count;
}

static int
tm_getopt_short(struct tm_getopt_s* options, const char* optstring)
{
	char* option = options->argv[options->optind];
	options->errmsg[0] = '\0';
	options->optopt = 0;
	options->optarg = 0;
	if (option == 0)
	{
		return -1;
	}
	else if (tm_getopt_is_dashdash(option))
	{
		options->optind++; /* consume "--" */
		return -1;
	}
	else if (!tm_getopt_is_shortopt(option))
	{
		if (options->permute)
		{
			int index = options->optind++;
			int r = tm_getopt_short(options, optstring);
			tm_getopt_permute(options, index);
			options->optind--;
			return r;
		}
		else
		{
			return -1;
		}
	}
	option += options->subopt + 1;
	options->optopt = option[0];
	int type = tm_getopt_argtype(optstring, option[0]);
	char* next = options->argv[options->optind + 1];
	switch (type)
	{
	case -1: {
		char str[2] = {0, 0};
		str[0] = option[0];
		options->optind++;
		return tm_getopt_error(options, TM_OPTPARSE_MSG_INVALID, str);
	}
	case TM_OPTPARSE_NONE:
		if (option[1])
		{
			options->subopt++;
		}
		else
		{
			options->subopt = 0;
			options->optind++;
		}
		return option[0];
	case TM_OPTPARSE_REQUIRED:
		options->subopt = 0;
		options->optind++;
		if (option[1])
		{
			options->optarg = option + 1;
		}
		else if (next != 0)
		{
			options->optarg = next;
			options->optind++;
		}
		else
		{
			char str[2] = {0, 0};
			str[0] = option[0];
			options->optarg = 0;
			return tm_getopt_error(options, TM_OPTPARSE_MSG_MISSING, str);
		}
		return option[0];
	case TM_OPTPARSE_OPTIONAL:
		options->subopt = 0;
		options->optind++;
		if (option[1])
			options->optarg = option + 1;
		else
			options->optarg = 0;
		return option[0];
	}
	return 0;
}

static int
tm_getopt_longopts_end(const struct tm_getopt_list_s* longopts, int i)
{
	return !longopts[i].longname && !longopts[i].shortname;
}

static void
tm_getopt_from_long(const struct tm_getopt_list_s* longopts, char* optstring)
{
	char* p = optstring;
	for (int i = 0; !tm_getopt_longopts_end(longopts, i); i++)
	{
		if (longopts[i].shortname && longopts[i].shortname < 127)
		{
			int a;
			*p++ = longopts[i].shortname;
			for (a = 0; a < (int)longopts[i].argtype; a++)
				*p++ = ':';
		}
	}
	*p = '\0';
}

/* Unlike strcmp(), handles options containing "=". */
static int
tm_getopt_longopts_match(const char* longname, const char* option)
{
	const char* a = option, *n = longname;
	if (longname == 0)
		return 0;
	for (; *a && *n && *a != '='; a++, n++)
		if (*a != *n)
			return 0;
	return *n == '\0' && (*a == '\0' || *a == '=');
}

/* Return the part after "=", or NULL. */
static char*
tm_getopt_longopts_arg(char* option)
{
	for (; *option && *option != '='; option++);
	if (*option == '=')
		return option + 1;
	else
		return 0;
}

static int
tm_getopt_long_fallback(struct tm_getopt_s* options, const struct tm_getopt_list_s* longopts, int* longindex)
{
	char optstring[96 * 3 + 1]; /* 96 ASCII printable characters */
	tm_getopt_from_long(longopts, optstring);
	int result = tm_getopt_short(options, optstring);
	if (longindex != 0)
	{
		*longindex = -1;
		if (result != -1)
		{
			for (int i = 0; !tm_getopt_longopts_end(longopts, i); i++)
				if (longopts[i].shortname == options->optopt)
					*longindex = i;
		}
	}
	return result;
}

int
tm_getopt(struct tm_getopt_s* options, const struct tm_getopt_list_s* longopts, int* longindex)
{
	char* option = options->argv[options->optind];
	if (option == 0)
	{
		return -1;
	}
	else if (tm_getopt_is_dashdash(option))
	{
		options->optind++; /* consume "--" */
		return -1;
	}
	else if (tm_getopt_is_shortopt(option))
	{
		return tm_getopt_long_fallback(options, longopts, longindex);
	}
	else if (!tm_getopt_is_longopt(option))
	{
		if (options->permute)
		{
			int index = options->optind++;
			int r = tm_getopt(options, longopts, longindex);
			tm_getopt_permute(options, index);
			options->optind--;
			return r;
		}
		else
		{
			return -1;
		}
	}

	/* Parse as long option. */
	options->errmsg[0] = '\0';
	options->optopt = 0;
	options->optarg = 0;
	option += 2; /* skip "--" */
	options->optind++;
	for (int i = 0; !tm_getopt_longopts_end(longopts, i); i++)
	{
		const char* name = longopts[i].longname;
		if (tm_getopt_longopts_match(name, option))
		{
			if (longindex)
				*longindex = i;
			options->optopt = longopts[i].shortname;
			char* arg = tm_getopt_longopts_arg(option);
			if (longopts[i].argtype == TM_OPTPARSE_NONE && arg != 0)
			{
				return tm_getopt_error(options, TM_OPTPARSE_MSG_TOOMANY, name);
			}
			if (arg != 0)
			{
				options->optarg = arg;
			}
			else if (longopts[i].argtype == TM_OPTPARSE_REQUIRED)
			{
				options->optarg = options->argv[options->optind];
				if (options->optarg == 0)
					return tm_getopt_error(options, TM_OPTPARSE_MSG_MISSING, name);
				else
					options->optind++;
			}
			return options->optopt;
		}
	}
	return tm_getopt_error(options, TM_OPTPARSE_MSG_INVALID, option);
}
