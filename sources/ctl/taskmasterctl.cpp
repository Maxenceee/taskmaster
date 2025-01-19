/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterctl.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:35 by mgama             #+#    #+#             */
/*   Updated: 2025/01/19 20:11:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tm.hpp"
#include "readline.hpp"

void
send_message(int sockfd, const char* message)
{
	if (send(sockfd, message, strlen(message), 0) < 0) {
		perror("send failed");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}

int
main(int argc, char* const* argv)
{
	// if (argc < 2 || argc > 3) {
	// 	std::cerr << "Usage: " << argv[0] << " <message>" << std::endl;
	// 	return 1;
	// }

	// int sockfd;
	// struct sockaddr_un servaddr;

	// // Create socket
	// if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	// 	perror("socket creation failed");
	// 	exit(EXIT_FAILURE);
	// }

	// // Set server address
	// memset(&servaddr, 0, sizeof(servaddr));
	// servaddr.sun_family = AF_UNIX;
	// strncpy(servaddr.sun_path, TM_SOCKET_PATH, sizeof(servaddr.sun_path) - 1);

	// // Connect to server
	// if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
	// 	perror("connect failed");
	// 	close(sockfd);
	// 	exit(EXIT_FAILURE);
	// }

	std::string input = readline("taskmasterctl> ");

	std::cout << "Input: " << input << std::endl;
	// send_message(sockfd, argv[1]);

	// Receive a message from the server
	// char buffer[1024];
	// int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
	// if (n < 0) {
	// 	perror("recv failed");
	// 	close(sockfd);
	// 	exit(EXIT_FAILURE);
	// }
	// buffer[n] = '\0';
	// std::cout << "Server: " << buffer << std::endl;

	// // Close the socket
	// close(sockfd);
	return 0;
}