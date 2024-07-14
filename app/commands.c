#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "commands.h"
#include "utils.h"
#include "map.h"

map global_map = {.map_size = 0};
uint64_t time_since_set_command = 0;
uint64_t time_since_get_command = 0;

void parse_resp(char *message, size_t length, char *output)
{

	int array_len = (int)strtok(message, "\r\n")[1] - '0';
	char commands[array_len + 1][COMMAND_MAX_LENGTH];

	for (int i = 0; i < array_len; i++) {
		int command_length = (int)strtok(NULL, "\r\n")[1] - '0';
		strncpy(commands[i], strtok(NULL, "\r\n"), command_length);
		commands[i][command_length] = '\0';
	}

	if (strcasecmp(commands[0], "PING") == 0)
		strcpy(output, "+PONG\r\n");
	else if (strcasecmp(commands[0], "ECHO") == 0) {
		if (commands[1] != NULL) {
			snprintf(output, BUFFER_SIZE, "$%zu\r\n%s\r\n", strlen(commands[1]), commands[1]);
			// char *result = (char *)malloc(100 * sizeof(char));
			// snprintf(result, 99, "$%zu\r\n%s\r\n", strlen(commands[1]), commands[1]);
			// strcpy(output, result);
			// free(result);
		}
		else {
			strcpy(output, "-SYNTAXERROR No parameter supplied for 'ECHO' command\r\n");
		}
	}
	else if (strcasecmp(commands[0], "SET") == 0)
	{
		field *f = (field *)malloc(sizeof(field));	
		if (array_len == 3) {
			strcpy(f->key, commands[1]);
			strcpy(f->value, commands[2]);
		}
		else if (array_len == 5) {
			strcpy(f->key, commands[1]);
			strcpy(f->value, commands[2]);
			if (strcasecmp(commands[3], "px") == 0) {
				f->expiry_time = atoi(commands[4]);	
				time_since_set_command = get_current_time();
			}
			else {
				strcpy(output, "-SYNTAXERROR Wrong format for 'SET' command\r\n");
				free(f);
				return;
			}
		}
		else {
			strcpy(output, "-SYNTAXERROR Insufficient parameters for 'SET' command\r\n");
			free(f);
			return;
		}

 
		global_map.bucket[global_map.map_size++] = f;
		strcpy(output, "+OK\r\n");
	}
	else if (strcasecmp(commands[0], "GET") == 0)
	{
		char *key = commands[1];
		// field *f = (field *)malloc(sizeof(field));
		field *f = NULL;

		for (int i = 0; i < global_map.map_size; i++) {
			if (strcmp(global_map.bucket[i]->key, key) == 0) {
				f = global_map.bucket[i];
				break;
			}
		}

		//if (f->value != NULL)
		if (f != NULL && f->value[0] != '\0')
		{
			uint64_t expiry_time = f->expiry_time;

			if (expiry_time != 0) {
				time_since_get_command = get_current_time();
				if ((time_since_get_command - time_since_set_command) < expiry_time) {	
					snprintf(output, BUFFER_SIZE, "$%zu\r\n%s\r\n", strlen(f->value), f->value);
				}
				else {
					strcpy(output, "$-1\r\n");
				}
			}
			else {
				snprintf(output, BUFFER_SIZE, "$%zu\r\n%s\r\n", strlen(f->value), f->value);
			}
		}
		else
		{
			strcpy(output, "$-1\r\n");
		}
	}
	else
	{
		strcpy(output, "-SYNTAXERROR unknown command\r\n");
	}
}