#include "server.h"
#include "options.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>

typedef struct {
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

struct server_options serv_opts;

int main(int argc, char *argv[]) {
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// default value for server options
	serv_opts.port = 6379;
	strcpy(serv_opts.replicaof, "");
	strcpy(serv_opts.replid, "8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb");
	serv_opts.repl_offset = 0;
	strcpy(serv_opts.master_host, "");
	serv_opts.master_port = 0;

	parse_arguments(&serv_opts, argc, argv);

	printf("server's port number: %d\n", serv_opts.port);
	printf("server's replica: %s\n\n", (strcmp(serv_opts.replicaof, "") != 0) ? serv_opts.replicaof : "no");
	printf("master server's host: %s\n", serv_opts.master_host);
	printf("master server's port: %d\n", serv_opts.master_port);

	if (strlen(serv_opts.master_host) != 0 && serv_opts.master_port != 0) {
		int master_fd;
		struct sockaddr_in master_serv_addr;
		struct hostent *master_serv;

		master_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (master_fd == -1) {
			printf("Master socket creation failed: %s...\n", strerror(errno));
			return 1;
		}

		master_serv = gethostbyname(serv_opts.master_host);
		if (master_serv == NULL) {
			fprintf(stderr, "ERROR, no such host\n");
			return 1;	
		}

		bzero((char *)&master_serv_addr, sizeof(master_serv_addr));
		master_serv_addr.sin_family = AF_INET;

		bcopy((char *) master_serv->h_addr, (char *)&master_serv_addr.sin_addr.s_addr, master_serv->h_length);
		master_serv_addr.sin_port = htons(serv_opts.master_port);
		
		if (connect(master_fd, (struct sockaddr *)&master_serv_addr, sizeof(master_serv_addr)) < 0) {
			printf("ERROR Failed to connect to master\n");
			close(master_fd);
			return 1;
		}

		send(master_fd, "*1\r\n$4\r\nPING\r\n", strlen("*1\r\n$4\r\nPING\r\n"), 0);
		printf("sent ping to master\n");
	}

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
		.sin_port = htons(serv_opts.port),
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
