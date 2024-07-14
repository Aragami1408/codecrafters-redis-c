#include <time.h>
#include <math.h>
#include "utils.h"

uint64_t get_current_time() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000 + round(spec.tv_nsec / 1.0e6);
}