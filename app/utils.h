#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

#define BUFFER_SIZE 1024
#define COMMAND_MAX_LENGTH 200
#define BUCKET_SIZE 10000

uint64_t get_current_time();
void strslice(const char* str, char* result, size_t start, size_t end);
void strprepend(char *s, const char *t);

#endif
