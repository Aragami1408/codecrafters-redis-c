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
#include <sys/time.h>
#include <time.h>
#include <inttypes.h>
#include <stdint.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define COMMAND_MAX_LENGTH 15
#define BUCKET_SIZE 10000

typedef struct
{
	char key[128];
	char value[128];
	uint64_t expiry_time;
} field;

typedef struct {
	field *bucket[BUCKET_SIZE];
	size_t map_size;
} map;

static map global_map = {.map_size = 0};
static uint64_t time_since_set_command = 0;
static uint64_t time_since_get_command = 0;


void *connection_handler(void *);
void parse_resp(char *message, size_t length, char *output);
uint64_t get_current_time();

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	int server_fd, client_fd, client_addr_len;
	struct sockaddr_in client_addr;
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(6379),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	pthread_t thread_id;

	while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len))) {
		puts("Client connected");

		if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_fd) < 0) {
			perror("Could not create thread");
			return 1;
		}

		puts("Handler assigned");
	}

	if (client_fd < 0) {
		printf("Accept failed: %s\n", strerror(errno));
		return 1;
	}

	close(server_fd);
	return 0;
}

void *connection_handler(void *fd)
{
	// Get the socket descriptor
	int client_fd = *(int *)fd;
	char client_message[BUFFER_SIZE];
	int bytes_received;

	while ((bytes_received = recv(client_fd, client_message, BUFFER_SIZE, 0)) > 0) {
		client_message[bytes_received] = '\0';
		printf("%s\n", client_message);

		char response[100];
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

void parse_resp(char *message, size_t length, char *output)
{
#define COMMAND_MAX_LENGTH 15

	int array_len = (int)strtok(message, "\r\n")[1] - '0';
	char commands[array_len + 1][COMMAND_MAX_LENGTH];

	for (int i = 0; i < array_len; i++) {
		int command_length = (int)strtok(NULL, "\r\n")[1] - '0';
		strncpy(commands[i], strtok(NULL, "\r\n"), command_length);
		commands[i][command_length] = '\0';
	}

	if (strcasecmp(commands[0], "PING") == 0)
		strcpy(output, "+PONG\r\n");
	else if (strcasecmp(commands[0], "ECHO") == 0) {
		if (commands[1] != NULL) {
			snprintf(output, BUFFER_SIZE, "$%zu\r\n%s\r\n", strlen(commands[1]), commands[1]);
			// char *result = (char *)malloc(100 * sizeof(char));
			// snprintf(result, 99, "$%zu\r\n%s\r\n", strlen(commands[1]), commands[1]);
			// strcpy(output, result);
			// free(result);
		}
		else {
			strcpy(output, "-SYNTAXERROR No parameter supplied for 'ECHO' command\r\n");
		}
	}
	else if (strcasecmp(commands[0], "SET") == 0)
	{
		field *f = (field *)malloc(sizeof(field));	
		if (array_len == 3) {
			strcpy(f->key, commands[1]);
			strcpy(f->value, commands[2]);
		}
		else if (array_len == 5) {
			strcpy(f->key, commands[1]);
			strcpy(f->value, commands[2]);
			if (strcasecmp(commands[3], "px") == 0) {
				f->expiry_time = atoi(commands[4]);	
				time_since_set_command = get_current_time();
			}
			else {
				strcpy(output, "-SYNTAXERROR Wrong format for 'SET' command\r\n");
				free(f);
				return;
			}
		}
		else {
			strcpy(output, "-SYNTAXERROR Insufficient parameters for 'SET' command\r\n");
			free(f);
			return;
		}

		global_map.bucket[global_map.map_size++] = f;
		strcpy(output, "+OK\r\n");
	}
	else if (strcasecmp(commands[0], "GET") == 0)
	{
		char *key = commands[1];
		// field *f = (field *)malloc(sizeof(field));
		field *f = NULL;

		for (int i = 0; i < global_map.map_size; i++) {
			if (strcmp(global_map.bucket[i]->key, key) == 0) {
				f = global_map.bucket[i];
				break;
			}
		}

		//if (f->value != NULL)
		if (f != NULL && f->value[0] != '\0')
		{
			uint64_t expiry_time = f->expiry_time;

			if (expiry_time != 0) {
				printf("The key %s will expire at %zu milliseconds\n", key, expiry_time);
				time_since_get_command = get_current_time();
				if ((time_since_get_command - time_since_set_command) < expiry_time) {	
					snprintf(output, BUFFER_SIZE, "$%zu\r\n%s\r\n", strlen(f->value), f->value);
				}
				else {
					strcpy(output, "$-1\r\n");
				}
			}
			else {
				printf("The key %s never expire\n", key);
				snprintf(output, BUFFER_SIZE, "$%zu\r\n%s\r\n", strlen(f->value), f->value);
			}
		}
		else
		{
			strcpy(output, "$-1\r\n");
		}
	}
	else
	{
		strcpy(output, "-SYNTAXERROR unknown command\r\n");
	}
}

uint64_t get_current_time() {
	struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
}