#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define BUFFER_SIZE 1024

typedef struct
{
	char key[128];
	char value[128];
} field;

typedef struct {
	field *bucket[10000];
	size_t map_size;
} map;

static map global_map;

// the thread callback
void *connection_handler(void *);

// RESP parsing function
void parse_resp(char *message, size_t length, char *output);

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Setup default values for hash table
	// hmput(hash, "foo", "bar");
	// hmput(hash, "baz", "qux");

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

	char client_message[BUFFER_SIZE];
	int bytes_received;
	while ((bytes_received = recv(client_fd, client_message, BUFFER_SIZE, 0)) > 0)
	{
		client_message[bytes_received] = '\0';
		printf("%s\n", client_message);

		char response[100];
		parse_resp(client_message, bytes_received, response);

		int bytes_sent = send(client_fd, response, strlen(response), 0);
	}

	if (bytes_received == 0)
	{
		printf("Client disconnected\n");
		fflush(stdout);
	}
	else if (bytes_received == -1)
	{
		printf("Receive failed: %s\n", strerror(errno));
	}

	return 0;
}

void parse_resp(char *message, size_t length, char *output)
{
#define COMMAND_MAX_LENGTH 15

	int array_len = (int)strtok(message, "\r\n")[1] - '0';

	char commands[array_len + 1][COMMAND_MAX_LENGTH];

	for (int i = 0; i < array_len; i++)
	{
		int command_length = (int)strtok(0, "\r\n")[1] - '0';
		strncpy(commands[i], strtok(0, "\r\n"), command_length);
		commands[i][command_length] = '\0';
	}

	if (strcasecmp(commands[0], "PING") == 0)
	{
		strcpy(output, "+PONG\r\n");
	}
	else if (strcasecmp(commands[0], "ECHO") == 0)
	{
		char *result = (char *)malloc(100 * sizeof(char));
		snprintf(result, 99, "$%zu\r\n%s\r\n", strlen(commands[1]), commands[1]);
		strcpy(output, result);
		free(result);
	}
	else if (strcasecmp(commands[0], "SET") == 0)
	{
		field *f = (field *)malloc(sizeof(field));	
		
		strcpy(f->key, commands[1]);
		strcpy(f->value, commands[2]);

		global_map.bucket[global_map.map_size++] = f;
		strcpy(output, "+OK\r\n");
		free(f);
	}
	else if (strcasecmp(commands[0], "GET") == 0)
	{
		field *f = (field *)malloc(sizeof(field));
		char *key = commands[1];
		for (int i = 0; i < global_map.map_size; i++) {
			if (strcmp(global_map.bucket[i]->key, key) == 0) {
				f = global_map.bucket[i];
				break;
			}
		}

		if (f->value != NULL)
		{
			char *value = f->value;
			char *result = (char *)malloc(100 * sizeof(char));
			snprintf(result, 99, "$%zu\r\n%s\r\n", strlen(value), value);
			strcpy(output, result);
			free(result);
		}
		else
		{
			strcpy(output, "$-1\r\n");
		}
		free(f);
	}
	else
	{
		strcpy(output, "-SYNTAX ERROR\r\n");
	}

#undef COMMAND_LENGTH
}