/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   taskmasterd.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:14:13 by mgama             #+#    #+#             */
/*   Updated: 2025/01/11 13:21:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "taskmaster.hpp"

// pid_t child_pid = -1;
// bool attach_output = false;

// void handle_sigusr1(int) {
//     // Attach child's output to parent's stdout
//     attach_output = true;
// }

// void handle_sigusr2(int) {
//     // Terminate the program
//     if (child_pid > 0) {
//         attach_output = false;
//     }
// }

// int
// main(int argc, char *const *argv, char *const *envp)
// {
//     // Set up signal handlers
//     signal(SIGUSR1, handle_sigusr1);
//     signal(SIGUSR2, handle_sigusr2);

//     // Start the child process
//     spawn_child(argv + 1, envp);

//     while (true) {
//         int pipe_fd[2];
//         if (attach_output) {
//             attach_output = false;
//             if (pipe(pipe_fd) == -1) {
//                 perror("pipe failed");
//                 continue;
//             }

//             // Spawn child with output redirection
//             if ((child_pid = fork()) == 0) {
//                 close(pipe_fd[0]); // Close read end
//                 spawn_child(argv, envp, pipe_fd[1]);
//                 close(pipe_fd[1]);
//                 exit(0);
//             } else {
//                 close(pipe_fd[1]); // Close write end
//                 char buffer[1024];
//                 ssize_t bytes_read;
//                 while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
//                     buffer[bytes_read] = '\0';
//                     std::cout << buffer;
//                 }
//                 close(pipe_fd[0]);
//             }
//         } else {
//             child_pid = fork();
//             if (child_pid == 0) {
// 				close(STDOUT_FILENO);
// 				close(STDERR_FILENO);
//                 spawn_child(argv, envp);
//                 exit(0);
//             }
//         }

//         // Parent waits for the child or handles signals
//         pause();
//     }

//     return 0;
// }

void
sigint_handler(int signum)
{
	(void)signum;
	unlink(WBS_SOCKET_PATH);
}

int
main()
{
	int sockfd;
	struct sockaddr_un addr;

	signal(SIGINT, sigint_handler);

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Erreur lors de la création de la socket");
		return (1);
	}

	int option = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
		return (1);
	}

	// Nettoyage de la structure d'adresse
	bzero(&addr, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, WBS_SOCKET_PATH, sizeof(addr.sun_path) - 1);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
		perror("Erreur lors de la liaison du socket à l'adresse");
		return (1);
	}

	printf("Socket Unix créée avec succès.\n");

	// Attente de connexion
	if (listen(sockfd, 5) == -1) {
		perror("Erreur lors de l'attente de connexion");
		return (1);
	}
	printf("En attente de connexion...\n");

	struct pollfd fds[1];
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	while (1) {
		int ret = poll(fds, 1, -1);
		if (ret == -1) {
			perror("Erreur lors de l'appel à poll");
			return (1);
		}

		if (fds[0].revents & POLLIN) {
			int client_sockfd;
			struct sockaddr_un client_addr;
			socklen_t client_addr_len = sizeof(struct sockaddr_un);

			client_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_addr_len);
			if (client_sockfd == -1) {
				perror("Erreur lors de l'acceptation de la connexion");
				return (1);
			}

			printf("Connexion acceptée.\n");

			// Réception des données
			char buffer[256];
			ssize_t bytes_received;

			bytes_received = recv(client_sockfd, buffer, sizeof(buffer), 0);
			if (bytes_received == -1) {
				perror("Erreur lors de la réception des données");
				return (1);
			} else {
				buffer[bytes_received] = '\0';
				printf("Données reçues : %s\n", buffer);
			}

			send(client_sockfd, "Message reçu", 13, 0); // Envoi d'un message de confirmation

			close(client_sockfd); // Fermeture de la connexion avec le client
		}
	}
	
	// Ton code peut continuer ici, par exemple en écoutant sur cette socket.
	unlink(WBS_SOCKET_PATH); // Supprime le socket Unix
	close(sockfd); // N'oublie pas de fermer la socket lorsque tu n'en as plus besoin

	return (0);
}

// int main(int argc, char *const *argv, char *const *envp)
// {
// 	(void)argc;
// 	// char *const load_args[] = {"/bin/echo", "Hello, world", NULL};
// 	// int fd = open("output.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

// 	if (spawn_child(argv + 1, envp) == -1) {
// 		return (EXIT_FAILURE);
// 	}
// }