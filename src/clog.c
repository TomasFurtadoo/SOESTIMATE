
#include "clog.h"
#include "main.h"
#include "memory.h"
#include "ctime.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

void log_operation(char *operation, struct info_container *info) {
    sem_wait(info->sems->log_mutex);
    FILE *f = fopen(info->log_filename, "a");
    if (f) {
        struct timespec ts;
        register_time(&ts);
        struct tm *lt = localtime(&ts.tv_sec);
        fprintf(f, "%04d%02d%02d %02d:%02d:%02d.%03ld %s\n",
                lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                lt->tm_hour, lt->tm_min, lt->tm_sec,
                ts.tv_nsec / 1000000, operation);
        fclose(f);
    }
    sem_post(info->sems->log_mutex);
}

void log_result (int server_id, struct MeasurementInfo *m, struct info_container *info){
    sem_wait(info ->sems->log_mutex);
    FILE *f = fopen(info->log_filename, "a");
    if (f) {
        struct timespec ts;
        register_time(&ts);
        struct tm *lt = localtime(&ts.tv_sec);

        double elapsed = (ts.tv_sec - m->change_time.main_time.tv_sec) +
                         (ts.tv_nsec - m->change_time.main_time.tv_nsec) / 1e9;

        fprintf(f, "%04d%02d%02d %02d:%02d:%02d.%03ld Server %d: %s\n",
                lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                lt->tm_hour, lt->tm_min, lt->tm_sec,
                ts.tv_nsec / 1000000, m->m_id, server_id, m->value, elapsed);
        fclose(f);
        *info->last_logged_id = m->m_id;
    }
    sem_post(info->sems->log_mutex);
}

void clear_log(struct info_container *info) {
    FILE *f = fopen(info->log_filename, "w");
    if (f) {
        fclose(f);
    }
}

double read_j(int m_id, struct info_container *info){
    sem_wait(info ->sems->log_mutex);
    FILE *f = fopen(info->log_filename, "r");
    if (!f) {
        sem_post(info->sems->log_mutex);
        return -1.0; // Error 
    }

    char line[512];
    double estimate = -1.0;
    while (fgets(line, sizeof(line), f)) {
        int id;
        char *res_ptr = strstr(line, " result ");
        if (res_ptr && sscanf(res_ptr + 8, "%d", &id) == 1) {
            if (id == m_id) {
                char *est_ptr = strstr(line, " estimate ");
                if (est_ptr && sscanf(est_ptr + 10, "%lf", &estimate) == 1) {
                    break; 
                }
            }
        }
    }
    fclose(f);
    sem_post(info->sems->log_mutex);
    return estimate;
}