#ifndef OPTIONS_H
#define OPTIONS_H

#include "utils.h"

struct server_options {
    int port;
    // replication
    char replicaof[BUFFER_SIZE];
    char replid[BUFFER_SIZE];
    int repl_offset;
    // master server info
    char master_host[BUFFER_SIZE];
    int master_port;
};

extern struct server_options serv_opts;

void parse_arguments(struct server_options *serv_opts, int argc, char **argv);
void print_usage(const char *program_name);

#endif