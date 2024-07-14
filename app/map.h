#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdlib.h>

#define BUCKET_SIZE 10000

typedef struct {
    char key[128];
    char value[128];
    uint64_t expiry_time;
} field;

typedef struct {
    field *bucket[BUCKET_SIZE];
    size_t map_size;
} map;

extern map global_map;
extern uint64_t time_since_set_command;
extern uint64_t time_since_get_command;

#endif
