#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <argp.h>


struct arguments {
	char *replicaof;
	int port;
};

extern struct argp argp;
extern struct arguments args;

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif