/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnixSocketServer.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:43:04 by mgama             #+#    #+#             */
/*   Updated: 2025/02/01 01:02:11 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "unix_socket/server/UnixSocketServer.hpp"
#include "logger/Logger.hpp"

bool check_living_socket(const char* socket_path);

UnixSocketServer::UnixSocketServer(const char* socket_path): UnixSocket(socket_path)
{
	// check_living_socket(socket_path);

	this->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (this->sockfd == -1)
	{
		Logger::perror("unix server: socket creation failed");
		throw std::runtime_error("socket creation failed");
	}

	int option = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1)
	{
		close(this->sockfd);
		Logger::perror("unix server: setsockopt failed");
		throw std::runtime_error("setsockopt failed");
	}

	(void)unlink(socket_path);
	bzero(&this->addr, sizeof(this->addr));
	this->addr.sun_family = AF_UNIX;
	strncpy(this->addr.sun_path, socket_path, sizeof(this->addr.sun_path) - 1);

	if (bind(this->sockfd, (struct sockaddr *) &this->addr, sizeof(struct sockaddr_un)) == -1)
	{
		if (errno == EADDRINUSE)
		{
			Logger::error("unix server: bind error: Address already in use");
		}
		else
		{
			Logger::perror("unix server: bind error");
		}
		throw std::runtime_error("could not bind socket to address");
	}
}

UnixSocketServer::~UnixSocketServer(void)
{
	close(this->sockfd);
	Logger::debug("Removing socket file: " + std::string(this->socket_path));
	unlink(this->socket_path);
}

int
UnixSocketServer::listen(void)
{
	if (::listen(this->sockfd, TM_DEFAULT_MAX_WORKERS) == -1)
	{
		Logger::perror("unix server: listen failed");
		return (TM_FAILURE);
	}
	this->poll_fds.push_back((pollfd){this->sockfd, TM_POLL_EVENTS, 0});
	this->_poll_clients[this->sockfd] = (tm_pollclient){TM_POLL_SERVER, nullptr};
	return (TM_SUCCESS);
}

int
read_from_client(int client_fd)
{
	char buffer[TM_RECV_SIZE];
	size_t bytes_received;

	bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
	if (bytes_received == -1)
	{
		Logger::perror("server error: an error occurred while receiving data from the client");
		return (TM_FAILURE);
	}
	else if (bytes_received == 0)
	{
		Logger::debug("Connection closed by the client");
		return (TM_FAILURE);
	}
	Logger::debug("Data received: " + std::string(buffer, bytes_received));
	return (TM_SUCCESS);
}

int
UnixSocketServer::poll(void)
{
	std::vector<int>	to_remove;

	if (::poll(this->poll_fds.data(), this->poll_fds.size(), TM_POLL_TIMEOUT) == -1)
	{
		if (errno == EINTR) {
			return (TM_SUCCESS);
		}
		Logger::perror("server error: an error occurred while poll'ing");
		return (TM_FAILURE);
	}

	for (size_t i = 0; i < this->poll_fds.size(); ++i)
	{
		if (this->poll_fds[i].revents & POLLHUP)
		{
			Logger::debug("Connection closed by the client (event POLLHUP)");
			to_remove.push_back(i);
		}
		else if (this->poll_fds[i].revents & POLLERR)
		{
			Logger::debug("Socket error (POLLERR detected)");
			to_remove.push_back(i);
		}
		else if (this->poll_fds[i].revents & POLLIN)
		{
			int newclient;

			switch (this->_poll_clients[this->poll_fds[i].fd].type)
			{
			case TM_POLL_SERVER:
				newclient = accept(this->sockfd, nullptr, nullptr);
				if (newclient == -1)
				{
					Logger::perror("server error: accept failed");
					return (TM_SUCCESS);
				}
				this->_poll_clients[newclient] = (tm_pollclient){TM_POLL_CLIENT, nullptr};
				this->poll_fds.push_back((pollfd){newclient, TM_POLL_EVENTS, 0});
				break;
			
			case TM_POLL_CLIENT:
				read_from_client(this->poll_fds[i].fd);
				close(this->poll_fds[i].fd);
				to_remove.push_back(i);
				break;
			}
		}
	}

	for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it)
	{
		close(this->poll_fds[*it].fd);
		this->poll_fds.erase(this->poll_fds.begin() + *it);
		this->_poll_clients.erase(this->poll_fds[*it].fd);
	}
	return (TM_SUCCESS);
}

bool
check_living_socket(const char* socket_path)
{
	struct stat buffer;
	if (stat(socket_path, &buffer) != 0)
	{
		return (false);
	}

	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1)
	{
		perror("socket");
		return (false);
	}

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0)
	{
		close(sock);
		return (true);
	}

	if (errno == ECONNREFUSED)
	{
		close(sock);
		unlink(socket_path);
		return (false);
	}

	close(sock);
	return (false);
}

int
init_and_start_unix_server()
{
	struct sockaddr_un addr;

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
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