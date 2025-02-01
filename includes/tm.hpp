/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tm.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 23:24:24 by mgama             #+#    #+#             */
/*   Updated: 2025/02/01 00:55:36 by mgama            ###   ########.fr       */
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

// CPP Includes
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

#define TM_VERSION "1.0"
#define TM_AUTHOR "mgama"

#define TM_PREFIX "taskmaster: "

#define TM_PID_FILE "./taskmaster.pid"
#define TM_SOCKET_PATH "./taskmaster.sock"

#define TM_SUCCESS 0
#define TM_FAILURE 1

#define TM_PIPE_READ_END 0
#define TM_PIPE_WRITE_END 1

// Global types

typedef std::chrono::steady_clock::time_point time_point;

#endif /* TM_HPP */