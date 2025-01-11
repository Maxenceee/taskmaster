/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unix_server.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:43:04 by mgama             #+#    #+#             */
/*   Updated: 2025/01/11 18:52:56 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "taskmaster.hpp"

void
unix_server_sigint_handler(int signum)
{
	(void)signum;
	unlink(TM_SOCKET_PATH);
}

int
init_and_start_unix_server()
{
	int sockfd;
	struct sockaddr_un addr;

	signal(SIGINT, unix_server_sigint_handler);

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
	strncpy(addr.sun_path, TM_SOCKET_PATH, sizeof(addr.sun_path) - 1);

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
	unlink(TM_SOCKET_PATH); // Supprime le socket Unix
	close(sockfd); // N'oublie pas de fermer la socket lorsque tu n'en as plus besoin

	return (0);
}