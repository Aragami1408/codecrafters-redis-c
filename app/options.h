#ifndef OPTIONS_H
#define OPTIONS_H

#include "utils.h"
#include "server.h"


extern struct server_info serv_info;

void parse_arguments(struct server_info *serv_info, int argc, char **argv);
void print_usage(const char *program_name);

#endif