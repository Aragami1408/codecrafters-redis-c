#include <time.h>
#include <math.h>
#include <string.h>
#include "utils.h"

uint64_t get_current_time() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
}

void strslice(const char* str, char* result, size_t start, size_t end) {
    strncpy(result, str + start, end - start);
}

void strprepend(char *s, const char *t) {
    size_t len = strlen(t);
    memmove(s+len, s, strlen(s) + 1);
    memcpy(s, t, len);
}