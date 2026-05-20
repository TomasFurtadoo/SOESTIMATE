#define _POSIX_C_SOURCE 200809L
#include "ctime.h"
#include <time.h>

struct timespec get_current_time(void){
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);

    return time;
}


double elapsed_time(struct timespec start, struct timespec end){
    double seconds = end.tv_sec - start.tv_sec;
    double nano_seconds = end.tv_nsec - start.tv_nsec;
    return seconds + (nano_seconds / 1e9);
    
}
