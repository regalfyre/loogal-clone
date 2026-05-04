#define _POSIX_C_SOURCE 200809L
#include "timer.h"
#include <time.h>

double loogal_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((double)ts.tv_sec * 1000.0) + ((double)ts.tv_nsec / 1000000.0);
}
