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

#define BUFFER_SIZE 1024

void *connection_handler(void *fd);

#endif