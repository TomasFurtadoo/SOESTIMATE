#include "cstats.h"
#include "main.h"
#include <stdio.h>
#include <unistd.h>


void clear_statistics(struct info_container *info){
    FILE *f = fopen(info->statistics_filename, "w");
    if (f){
        close(f);
    }

}


void write_statistics(struct info_container *info){
    FILE *f = fopen(info->statistics_filename, "a");
    #define PRINT_BOTH(...) do { \
        printf(__VA_ARGS__); \
        if (f){ \
            fprintf(f, __VA_ARGS__); \
        } \
    } while(0)

    PRINT_BOTH("===== FINAL STATISTICS =====\n");
    PRINT_BOTH("Main PID: %d\n", getpid());
    PRINT_BOTH("n_sensors=%d, n_servers=%d, buffers_size=%d\n", 
               info->n_sensors, info->n_servers, info->buffers_size);
    PRINT_BOTH("total_measurements=%d\n", *info->total_measurements);
    
    PRINT_BOTH("Sensors:\n");
    for (int i = 0; i < info->n_sensors; i++) {
        PRINT_BOTH(" Sensor %d (PID %d): generated=%d\n", 
                   i, info->sensors_pids[i], info->num_generated_measurements[i]);
    }

    PRINT_BOTH("Controllers:\n");
    for (int i = 0; i < info->n_sensors; i++) {
        PRINT_BOTH("controller=%d pid=%d\n", 
                   i, info->controllers_pids[i]);
    }

    PRINT_BOTH("Servers:\n");
    for (int i = 0; i < info->n_servers; i++) {
        PRINT_BOTH(" Server %d (PID %d): estimates=%d\n", 
                   i, info->servers_pids[i], info->num_estimates[i]);
    }

    PRINT_BOTH("Last logged ID: %d\n", *info->last_logged_id);
    PRINT_BOTH("============================\n");

        if (f){
            fclose(f);
        }

}