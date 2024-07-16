#ifndef OPTIONS_H
#define OPTIONS_H

#include "utils.h"

struct server_options {
    int port;
    char replicaof[BUFFER_SIZE];
};

extern struct server_options serv_opts;

void print_usage(const char *program_name);

#endif