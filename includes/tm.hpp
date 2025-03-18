/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tm.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 23:24:24 by mgama             #+#    #+#             */
/*   Updated: 2025/03/18 11:53:45 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TM_HPP
#define TM_HPP

// Std
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <spawn.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>

// Sys
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/resource.h>

// Net
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netdb.h>
#include <poll.h>
#include <arpa/inet.h>

// CPP Std
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <ctime>
#include <limits>
#include <cstdio>
#include <cstdarg>
#include <filesystem>
#include <numeric>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstddef>
#include <optional>
#include <thread>

// CPP Containers
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iterator>
#include <list>
#include <utility>
#include <queue>
#include <deque>

// 

#define TM_PROJECT "taskmaster"
#define TM_VERSION "1.0"
#define TM_AUTHOR "mgama"

#define TM_PREFIX TM_PROJECT ": "

#define TM_PID_FILE "./taskmaster.pid"
#define TM_SOCKET_PATH "unix://./taskmaster.sock"

#define TM_INT32_LEN   (sizeof("-2147483648") - 1)
#define TM_INT64_LEN   (sizeof("-9223372036854775808") - 1)

#define TM_DEFAULT_FILE_MODE 0644

#define TM_SUCCESS 0
#define TM_FAILURE 1

#define TM_PIPE_READ_END 0
#define TM_PIPE_WRITE_END 1

#define TM_OCTO "              .~~~.         ,,\n   ,,        /     \\       ;,,'\n  ;, ;      (  -  - )      ; ;\n    ;,';,,,  \\  \\/ /      ,; ;\n ,,,  ;,,,,;;,`   '-,;'''',,,'\n;,, ;,, ,,,,   ,;  ,,,'';;,,;''';\n   ;,,,;    ~~'  '';,,''',,;''''\n                      '''"

// Global types

typedef std::chrono::steady_clock::time_point time_point;

#endif /* TM_HPP */