#ifndef SERVER_H
#define SERVER_H

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
#include "utils.h"

struct server_info {
    int port;
    // replication
    char replicaof[BUFFER_SIZE];
    char replid[BUFFER_SIZE];
    int repl_offset;
    // master server info
    char master_host[BUFFER_SIZE];
    int master_port;
};

void *connection_handler(void *fd);

#endif