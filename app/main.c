#include "server.h"
#include "arguments.h"

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

struct argp_option options[] = {
	{"replicaof",   'r', "REPLICAOF", 0,  "replica of master (default: \"\")" },
	{"port",	 'p', "PORT",			0,  "master port (default: 6379)" },
	{ 0 }
};

struct argp argp = {options, parse_opt, "", ""};
struct arguments args;

int main(int argc, char *argv[]) {
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


	args.port = 6379;
	argp_parse(&argp, argc, argv, 0, 0, &args);

	printf("server's port: %d\nreplica: %s\n", args.port, (args.replicaof != NULL) ? args.replicaof : "no");

	// if (argc == 1)
	// 	port_number = 6379;
	// else if (argc == 3) {
	// 	if (strcmp(argv[1], "--port") == 0 || strcmp(argv[1], "-p") == 0) {
	// 		char *endptr;
	// 		port_number = strtol(argv[2], &endptr, 10);
	// 		if (endptr == argv[2] || *endptr != '\0') {
	// 			printf("Invalid port name\n");
	// 			printf("Usage: %s [--port <port number>]", argv[0]);
	// 			return 1;
	// 		}
	// 	}	
	// 	else {
	// 		printf("Usage: %s [--port <port number>]", argv[0]);
	// 		return 1;
	// 	}
	// }
	// else {
	// 	printf("Usage: %s [--port <port number>]", argv[0]);
	// 	return 1;
	// }

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(args.port),
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

