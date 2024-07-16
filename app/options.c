#include "options.h"

#include <stdio.h>

void print_usage(const char *program_name) {
	printf("Usage: %s [--port <port number>] [--replicaof <address>] [--help]\n", program_name);
}