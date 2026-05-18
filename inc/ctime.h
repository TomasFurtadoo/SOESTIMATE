#ifndef CTIME_H_GUARD
#define CTIME_H_GUARD
#include <time.h>

struct timestamps {
    struct timespec main_time;
    struct timespec sensor_time;
    struct timespec controller_time;
    struct timespec server_time;
};

struct timespec get_current_time (void);

double elapsed_time(struct timespec start, struct timespec end);

#endif