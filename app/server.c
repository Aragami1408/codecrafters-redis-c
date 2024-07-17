#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "server.h"
#include "commands.h"
#include "utils.h"

void *connection_handler(void *fd)
{
	// Get the socket descriptor
	int client_fd = *(int *)fd;
	char client_message[BUFFER_SIZE];
	int bytes_received;

	while ((bytes_received = recv(client_fd, client_message, BUFFER_SIZE, 0)) > 0) {
		client_message[bytes_received] = '\0';
		printf("%s\n", client_message);

		char response[BUFFER_SIZE];
		parse_resp(client_message, bytes_received, response);

		send(client_fd, response, strlen(response), 0);
	}

	if (bytes_received == 0) {
		printf("Client disconnected\n");
		fflush(stdout);
	}
	else if (bytes_received == -1) {
		printf("Receive failed: %s\n", strerror(errno));
	}

	close(client_fd);
	return 0;
}