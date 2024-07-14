#ifndef SERVER_H
#define SERVER_H

#include "utils.h"

#define BUFFER_SIZE 1024

void *connection_handler(void *fd);

#endif