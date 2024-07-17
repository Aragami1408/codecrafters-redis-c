#include "options.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parse_arguments(struct server_info *serv_opts, int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--port") == 0) {
			if (i+1 < argc) {
				char *endptr;
				serv_opts->port = strtol(argv[i+1], &endptr, 10);
				if (endptr == argv[i+1] || *endptr != '\0') {
					printf("Invalid port number: %s\n", argv[i + 1]);
					print_usage(argv[0]);
					exit(1);
				}
				i++;
			}
			else {
				printf("Option --port requires an argument\n");
				print_usage(argv[0]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--replicaof") == 0) {
			if (i+1 < argc) {
				strcpy(serv_opts->replicaof, argv[i+1]);
				strcpy(serv_opts->master_host, strtok(argv[i+1], " "));
				serv_opts->master_port = atoi(strtok(NULL, " "));
				i++;	
			}
			else {
				printf("Option --replicaof requires an argument\n");
				print_usage(argv[0]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0]);
			exit(0);
		}
		else {
			printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            exit(1);
		}
	}
}

void print_usage(const char *program_name) {
	printf("Usage: %s [--port <port number>] [--replicaof <address>] [--help]\n", program_name);
}