#define _POSIX_C_SOURCE 200809L
#include "main.h"
#include "csignal.h"
#include <signal.h>
#include "ctime.h"
#include <stdio.h>
#include <unistd.h>

static void sigint_handler(int signum){
    end_execution(info, buffs);
}

void setup_sigint_handler(void){
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

void ignore_signals(void){
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

void alarm_handler(int signum){
    alarm(info->period);
    struct timespec current_time = get_current_time();

    //main sensors
    for(int i = 0; i < info->buffers_size; i++){
        
        MeasurementInfo *m = &buffs->buff_main_sensors->buffer[i];
        if(m->m_id > 0){
            double decorrido;
            if(m->change_time.server_time.tv_sec != 0){
                decorrido = elapsed_time(m->change_time.main_time, m->change_time.server_time);
            } 
            else{
                decorrido = elapsed_time(m->change_time.main_time, current_time);
            }
            printf("%d %.3f\n", m->m_id, decorrido);
        }
    }

    //sensors controllers
    for(int i = 0; i < info->n_sensors * info->buffers_size; i++){
        MeasurementInfo *m = &buffs->buff_sensors_controllers->buffer[i];
        if(m->m_id > 0){
            double decorrido;
            if(m->change_time.server_time.tv_sec != 0){
                decorrido = elapsed_time(m->change_time.main_time, m->change_time.server_time);
            }
            else{
                decorrido = elapsed_time(m->change_time.main_time, current_time);
            }
            printf("%d %.3f\n", m->m_id, decorrido);
        }
    }

    //controllers servers
    for(int i = 0; i < info->n_sensors * info->buffers_size; i++){
        MeasurementInfo *m = &buffs->buff_controllers_servers->buffer[i];
        if(m->m_id > 0){
            double decorrido;
            if(m->change_time.server_time.tv_sec != 0){
                decorrido = elapsed_time(m->change_time.main_time, m->change_time.server_time);
            }
            else{
                decorrido = elapsed_time(m->change_time.main_time, current_time);
            }
            printf("%d %.3f\n", m->m_id, decorrido);
        }
    }

}

void setup_alarm(int period){
    if(period > 0){
        struct sigaction sa;
        sa.sa_handler = alarm_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGALRM, &sa, NULL);
        alarm(period);
    }
}













