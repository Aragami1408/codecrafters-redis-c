#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

#define COMMAND_MAX_LENGTH 15

void parse_resp(char *message, size_t length, char *output);

#endif
