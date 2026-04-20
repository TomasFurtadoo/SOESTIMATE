#include "server.h"
#include "memory.h"
#include "main.h"
#include "random_measurement.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int execute_server(int server_id, struct info_container *info, struct buffers *buffs){
    int expected_m_id = 1;
    double estimate;
    int valid_count;
    int received_flags[info->n_sensors];
    
    while(*info->terminate != 1){   
        estimate = 0;
        valid_count = 0;
        int received_count = 0;
        memset(received_flags, 0, sizeof(received_flags));
        
        while(received_count < info->n_sensors && *info->terminate != 1){
            MeasurementInfo m;
            read_expected_cycle_measurement(&m, expected_m_id, received_flags, info, buffs);

            if(m.m_id != -1){
                int success = server_process_measurement(&m, server_id, info, &estimate, &valid_count);

                if(success){
                    received_flags[m.controller_id] = 1;
                    received_count++;
                }
                else {
                    printf("[Server %d] Error processing measurement from Controller %d\n", server_id, m.controller_id);
                }
            }
            else{
                usleep(1000);
            }
        }
        if(received_count == info->n_sensors){
            if(valid_count > 0){
                estimate = estimate / valid_count; 
            } else {
                estimate = 0; 
            }

            info->num_estimates[server_id]++;
            server_print_estimate(server_id, expected_m_id, estimate, valid_count);

            expected_m_id++;
        }
    }
    return 0;
}

void read_expected_cycle_measurement(MeasurementInfo *m, int expected_m_id, const int *received_flags, struct info_container *info, struct buffers *buffs){
    m->m_id = -1;
    for(int i = 0; i < info->n_sensors; i++){
        if(received_flags[i] == 0){
            read_controller_servers_buffer(buffs->buff_controllers_servers, info->buffers_size, expected_m_id, i, m);
            if(m->m_id != -1) return;
        }
    }
}

int server_process_measurement(MeasurementInfo *m, int server_id, struct info_container *info, double *estimate, int *valid_count){
    if (m == NULL || info == NULL || estimate == NULL || valid_count == NULL) {
        return 0; 
    }

    if(m->m_id < 0){
        return 0;
    }

    if (m->state == VALID) {
        *estimate += m->value;
        (*valid_count)++;
    }
    
    return 1;

}

void server_print_estimate(int server_id, int m_id, double estimate, int valid_count){
    if(valid_count == 0) {
        // Mensagem quando não há dados válidos para a média
        printf("[Server %d] Cycle %d all measurements were invalid\n", server_id, m_id); // 
    }
    else {
        // Mensagem de sucesso com a estimativa formatada com 4 casas decimais
        printf("[Server %d] New estimate: cycle %d = %.4f (valid_count=%d)\n", 
                server_id, m_id, estimate, valid_count); // 
    }
}