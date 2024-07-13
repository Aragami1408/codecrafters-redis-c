#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// the thread function
void *connection_handler(void *);

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(6379),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	pthread_t thread_id;
	int client_fd;
	while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)))
	{
		printf("Client connected\n");

		if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_fd) < 0)
		{
			perror("Could not create thread");
			return 1;
		}

		puts("Handler assigned");
	}

	if (client_fd < 0)
	{
		printf("Accept failed: %s\n", strerror(errno));
		return 1;
	}

	close(server_fd);
	close(client_fd);
	return 0;
}

void *connection_handler(void *fd)
{
	// Get the socket descriptor
	int client_fd = *(int *)fd;

	char buffer[BUFFER_SIZE];
	int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (bytes_received == -1)
	{
		printf("Receive failed: %s\n", strerror(errno));
		return 1;
	}
	if (bytes_received == 0)
	{
		printf("Client disconnected\n");
		return 1;
	}
	buffer[bytes_received] = '\0';
	printf("Buffer Received: %s\n", buffer);

	const char *response = "+PONG\r\n";
	int bytes_sent = send(client_fd, response, strlen(response), 0);
	if (bytes_sent < 0)
	{
		printf("Send failed: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}